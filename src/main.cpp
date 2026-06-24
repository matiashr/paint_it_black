// pitb - paint it black: colorize keywords read from stdin.
//
// Keywords default to a hardcoded set, but ~/.pitb (if present) is loaded
// to add or override keyword -> color mappings. The file format is one
// "keyword; color" pair per line, e.g.:
//
//     while; white
//     for;green
//     entity; black
//
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cctype>
#include <string>
#include <vector>

// Map a human color name to its ANSI SGR escape sequence.
struct Color {
	const char* name;
	const char* code;
};
static const Color kColors[] = {
	{"black",   "\033[30m"},
	{"red",     "\033[31m"},
	{"green",   "\033[32m"},
	{"yellow",  "\033[33m"},
	{"blue",    "\033[34m"},
	{"magenta", "\033[35m"},
	{"cyan",    "\033[36m"},
	{"white",   "\033[37m"},
	{NULL, NULL},
};

static const char* ansi_for(const char* name)
{
	for (int i = 0; kColors[i].name != NULL; i++) {
		if (strcmp(name, kColors[i].name) == 0)
			return kColors[i].code;
	}
	return NULL;
}

struct Keyword {
	std::string word;
	std::string code; // ANSI escape for the color
};
static std::vector<Keyword> g_keywords;

// Insert or override the color for a keyword.
static void set_keyword(const std::string& word, const char* code)
{
	for (size_t i = 0; i < g_keywords.size(); i++) {
		if (g_keywords[i].word == word) {
			g_keywords[i].code = code;
			return;
		}
	}
	Keyword k;
	k.word = word;
	k.code = code;
	g_keywords.push_back(k);
}

static void load_defaults()
{
	set_keyword("if",    ansi_for("blue"));
	set_keyword("while", ansi_for("yellow"));
	set_keyword("void",  ansi_for("yellow"));
	set_keyword("{",     ansi_for("yellow"));
	set_keyword("}",     ansi_for("yellow"));
}

static std::string trim(const std::string& s)
{
	size_t a = 0, b = s.size();
	while (a < b && isspace((unsigned char)s[a])) a++;
	while (b > a && isspace((unsigned char)s[b - 1])) b--;
	return s.substr(a, b - a);
}

// Load ~/.pitb if it exists. Returns silently when absent.
static void load_config()
{
	const char* home = getenv("HOME");
	if (home == NULL)
		return;

	std::string path = std::string(home) + "/.pitb";
	FILE* f = fopen(path.c_str(), "r");
	if (f == NULL)
		return;

	char line[1024];
	while (fgets(line, sizeof(line), f) != NULL) {
		std::string s = trim(line);
		if (s.empty() || s[0] == '#')
			continue;

		size_t sep = s.find(';');
		if (sep == std::string::npos)
			continue;

		std::string word = trim(s.substr(0, sep));
		std::string color = trim(s.substr(sep + 1));
		if (word.empty() || color.empty())
			continue;

		const char* code = ansi_for(color.c_str());
		if (code == NULL) {
			fprintf(stderr, "pitb: unknown color '%s' for keyword '%s'\n",
			        color.c_str(), word.c_str());
			continue;
		}
		set_keyword(word, code);
	}
	fclose(f);
}

// Print a token, wrapped in color if it is a known keyword.
static void colorize(const std::string& word)
{
	if (word.empty())
		return;
	for (size_t i = 0; i < g_keywords.size(); i++) {
		if (g_keywords[i].word == word) {
			printf("%s%s\033[0m", g_keywords[i].code.c_str(), word.c_str());
			return;
		}
	}
	fputs(word.c_str(), stdout);
}

static bool is_word_char(int c)
{
	return isalnum(c) || c == '_';
}

int main(int argc, char* argv[])
{
	(void)argc;
	(void)argv;

	load_defaults();
	load_config();

	std::string word;
	int c;
	while ((c = getchar()) != EOF) {
		if (is_word_char(c)) {
			word.push_back((char)c);
		} else {
			// Flush the accumulated word, then handle the
			// separator on its own (so e.g. '{' can match too).
			colorize(word);
			word.clear();
			colorize(std::string(1, (char)c));
		}
	}
	colorize(word);
	return 0;
}
