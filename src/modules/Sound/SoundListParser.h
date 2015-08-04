#ifndef SOUNDLISTPARSER_H
#define SOUNDLISTPARSER_H
#include <vector>
#include <string>

// Takes an input string in the form: sound1,sound2,242,sound4 and generates a list of files needed to construct that sound

class SoundListParser{
private:
    std::vector<std::string> mList;

public:
	SoundListParser(std::string url);
	~SoundListParser();

    std::vector<std::string>& getSoundList();
};

#endif

