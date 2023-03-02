#include <algorithm>
#include <cctype>
#include <fstream>
#include <iostream>
#include <regex>
#include <set>
#include <sstream>
#include <string>
#include <string_view>
#include <vector>

std::string get_arg_param(const std::vector<std::string>& args, std::string_view expected_arg)
{
	auto itr = std::find(args.begin(), args.end(), expected_arg);
	if (itr == args.end())
		return "";
	itr++;
	if (itr == args.end())
		return "";
	std::string param = *itr;
	if (param.at(0) != '-')
		return param;
	return "";
}

std::vector<std::string> split(const std::string& s, const char delim)
{
	std::vector<std::string> result;
	std::stringstream ss (s);
	std::string token;
	while (getline(ss, token, delim))
	{
		result.push_back(token);
	}
	return result;
}

std::vector<std::string> filterWordsWithoutIncludedLetters(
	const std::vector<std::string>& wordList,
	const int wordLength,
	const std::string& includeArg)
{
	if (wordList.empty() || wordLength < 2 || includeArg.length() == 0)
		return wordList;
	
	std::set<char> includedLetters;
	std::vector<std::string> includeArgs = split(includeArg, ',');
	for (const std::string& arg : includeArgs)
	{
		if (arg.length() != 1)
			continue;
		const char c = arg.at(0);
		if (std::isalpha(c))
			includedLetters.insert(c);
	}

	if (includedLetters.size() == 0 || includedLetters.size() > wordLength)
		return wordList;

	std::vector<std::string> filteredWords;
	for (const std::string& word : wordList)
	{
		bool wordIsValid = true;
		for (const char c : includedLetters)
		{
			if (word.find(c) == std::string::npos)
			{
				wordIsValid = false;
				break;
			}
		}
		if (wordIsValid)
			filteredWords.push_back(word);
	}

	if (filteredWords.size() < wordList.size())
		return filteredWords;
	return wordList;
}


int main(int argc, char** argv)
{
	if (argc < 2)
	{
		std::cerr << "Error: No arguments were provided.\n";
		return EXIT_FAILURE;
	}
	
	// -----------------------------
	// PARSE COMMAND LINE ARGUMENTS
	// -----------------------------
	std::vector<std::string> args(argv + 1, argv + argc);
	
	// Show how the user's arguments were interpreted.
	const bool verbose = std::find(args.begin(), args.end(), "--verbose") != args.end();
	
	// Path to text file containing a list of words.
	const std::string wordFilePathParam = get_arg_param(args, "-list");
	const std::string wordFilePath = wordFilePathParam != "" ? wordFilePathParam: "wordlewords.txt";
	
	// The length of the word to be found.
	int tempWordLength = 5;
	const std::string wordLengthParam = get_arg_param(args, "-length");
	if (wordLengthParam != "")
	{
		try
		{
			const int wordLengthParamValue = std::stoi(wordLengthParam);
			tempWordLength = wordLengthParamValue;
		}
		catch(std::invalid_argument const& ex)
		{
			std::cerr << "Invalid parameter to -length argument: " << ex.what() << "\n";
		}
		catch(std::out_of_range const& ex)
		{
			std::cerr << "Parameter to -length argument was out of range: " << ex.what() << "\n";
		}
	}
	const int wordLength = tempWordLength;
	
	// List of letters known to not be in the word.
	// Separate multiple with a comma: -exclude m,s,e
	const std::string excludeArg = get_arg_param(args, "-exclude");
	
	// List of letters known to be in the word but whose positions are unknown.
	// Separate multiple with a comma: -include m,s,e
	const std::string includeArg = get_arg_param(args, "-include");
	
	// List of known positions and letters.
	// Separate multiple with a comma: -known 1m,2o,3u
	const std::string knownArg = get_arg_param(args, "-known");
	
	// Save the potential solutions in a .txt file.
	const bool saveToTxt = std::find(args.begin(), args.end(), "--save") != args.end();

	// ------------------------
	// VALIDATE USER ARGUMENTS
	// ------------------------
	if (wordLength < 2)
	{
		std::cerr << "Error: Word length must be at least 2.\n";
		return EXIT_FAILURE;
	}
	if (excludeArg.length() == 0 && includeArg.length() == 0 && knownArg.length() == 0)
	{
		std::cerr << "Error: No valid parameters were found for any of the options.\n";
		return EXIT_FAILURE;
	}
	
	std::regex posRegex("(\\d+)([a-z])");
	std::smatch posMatches;

	// ------------------
	// GET VALID LETTERS
	// ------------------
	// Use set to prevent duplicate letters from appearing
	std::set<char> excludedLetterSet;
	std::vector<std::string> excludeArgs = split(excludeArg, ',');
	for (const std::string& arg : excludeArgs)
	{
		if (arg.length() != 1)
			continue;
		const char c = arg.at(0);
		if (std::isalpha(c))
			excludedLetterSet.insert(c);
	}
	
	// Now use the set to create a string of valid letters
	if (excludedLetterSet.size() >= 26)
	{
		std::cerr << "Error: All letters of the alphabet have been excluded.";
		return EXIT_FAILURE;
	}
	std::string letterGroup = "[";
	if (excludedLetterSet.size() > 0)
	{
		letterGroup += "^";
		for (const char c : excludedLetterSet)
		{
			letterGroup += c;
		}
	}
	else
	{
		letterGroup += "a-z";
	}
	letterGroup += "]";
	
	if (verbose)
	{
		std::cout << "Regex letter group for unknown positions:\n";
		std::cout << letterGroup << "\n\n";
	}
	
	// --------------------
	// GET KNOWN POSITIONS
	// --------------------
	std::vector<std::string> knownArgs = split(knownArg, ',');
	std::vector<char> knownPositions(wordLength);
	int numKnownPositions = 0;
	for (std::size_t i = 0; i < wordLength; i++)
	{
		knownPositions[i] = '*';
	}
	for (const std::string& arg : knownArgs)
	{
		if (std::regex_match(arg, posMatches, posRegex))
		{
			const int position = std::stoi(posMatches[1]);
			if (position < 1 || position > wordLength)
				continue;
			const std::string charStr = posMatches[2];
			knownPositions[position - 1] = charStr.at(0);
			numKnownPositions++;
		}
	}

	// --------------------
	// BUILD REGEX PATTERN
	// --------------------
	std::string regexString = "";
	if (numKnownPositions == 0)
	{
		regexString = letterGroup + "{" + std::to_string(wordLength) + "}";
	}
	else
	{
		for (const char c : knownPositions)
		{
			if (c != '*')
				regexString += c;
			else
				regexString += letterGroup;
		}
	}
	regexString = "^" + regexString + "$";
	if (verbose)
	{
		std::cout << "Regex pattern to apply to each word:\n";
		std::cout << regexString << "\n\n";
	}
	std::regex wordleRegex(regexString);

	// ---------------------------------
	// APPLY ARGUMENTS TO WORDS IN FILE
	// ---------------------------------
	std::ifstream wordFile;
	wordFile.open(wordFilePath);
	if (!wordFile.is_open())
	{
		std::cerr << "Error when trying to open \"" << wordFilePath << "\".\n";
		return EXIT_FAILURE;
	}
	std::vector<std::string> wordList;
	std::string line;
	while (std::getline(wordFile, line))
	{
		std::transform(line.begin(), line.end(), line.begin(), ::tolower);
		if (line.length() == wordLength && std::regex_match(line, wordleRegex))
			wordList.push_back(line);
	}
	wordFile.close();

	wordList = filterWordsWithoutIncludedLetters(
		wordList,
		wordLength,
		includeArg
	);
	
	// -------------
	// SHOW RESULTS
	// -------------
	std::cout << wordList.size() << " possible solutions:\n";
	for (const std::string& word : wordList)
	{
		std::cout << word << std::endl;
	}
	
	if (saveToTxt)
	{
		std::ofstream txtFile("results.txt");
		for (const std::string& word : wordList)
		{
			txtFile << word << std::endl;
		}
		txtFile.close();
	}
}
