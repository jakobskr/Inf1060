#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#define CNRM "\x1b[0m"
#define CRED "\x1b[31m"
#define CGRN "\x1b[32m"


int stringsum(char* s);
void stringsum2(char *s, int *res);
int distance_between(char *s, char c);
char *string_between(char *s, char c);
char** split(char* s);

static int test_num = 1;

static void logger(int passed, char *s)
{
	char *res;
	char *color;

	if (passed) {
		res = "PASS";
		color = CGRN;
	} else {
		res = "FAIL";
		color = CRED;
	}
	printf("[Test %d][%s%s%s] %s\n", test_num++, color, res, CNRM, s);
}

static void test_stringsum(char *input, int expected)
{
	int test;
	char buf[256] = { 0 };

	test = stringsum(input);
	sprintf(buf, "Returned: %d, Expected: %d", test, expected);
	logger(test == expected, buf);
}

static void test_split(char *str, char **correct)
{
	int i, pass = 1;
	char buf[512] = { 0 };

	// Allow mutation of original string
	size_t len = strlen(str) + 1;
	char *str_cpy = malloc(len);
	snprintf(str_cpy, len, "%s", str);

	char **res = split(str_cpy);

	if (!res || !res[0]) {
		pass = 0;
		sprintf(buf, "split() returned NULL or an empty array");
		goto end;
	}

	for (i = 0; correct[i]; i++) {
		if (!res[i]) {
			pass = 0;
			sprintf(buf, "split() returned fewer words than expected");
			goto end;
		}
	}

	if (res[i]) {
		pass = 0;
		sprintf(buf, "split() returned more words than expected");
		goto end;
	}

    if (strcmp(str, str_cpy)) {
		pass = 0;
		sprintf(buf, "split() modified the original string!");
		goto end;
    }

end:
	sprintf(buf, "\n%-16s%-16s\n", "Returned", "Expected");

	for (i = 0; res[i]; i++) {
		char tmp[256] = { 0 };
		sprintf(tmp, "%-16s%-16s\n", res[i], correct[i]);
		strcat(buf, tmp);
		if (strcmp(res[i], correct[i])) {
			pass = 0;
            break;
		}
	}

	logger(pass, buf);
	free(str_cpy);
}

static void test_distance_between(char *str, char c, int expected)
{
	int test;
	char buf[256] = { 0 };

	test = distance_between(str, c);
	sprintf(buf, "Returned: %d, Expected: %d", test, expected);
	logger(test == expected, buf);
}


static void test_string_between(char *str, char c, const char *expected)
{
	char *res_char;
	char buf[256] = { 0 };

	res_char = string_between(str, c);
	snprintf(buf, sizeof(buf), "Returned: %s, Expected: %s", res_char, expected);

	if (!res_char && expected) {
		logger(0, buf);
	} else {
		if (!expected)
			logger(!res_char, buf);
		else
			logger(!strcmp(res_char, expected), buf);
		free(res_char);
	}
}

static void test_stringsum2(char *input, int expected)
{
	int res_int;
	char buf[256] = { 0 };

	stringsum2(input, &res_int);
	sprintf(buf, "Returned: %d, Expected: %d", res_int, expected);
	logger(res_int == expected, buf);
}

int main(void)
{
	
	printf("Testing stringsum()\n");
	test_stringsum("abcd", 10);
	test_stringsum("a!", -1);
	test_stringsum("aAzZ", 54);
	test_stringsum("ababcDcabcddAbcDaBcabcABCddabCddabcabcddABCabcDd", 120);
	test_stringsum("", 0);

	test_num = 1;
	printf("\nTesting distance_between()\n");
	test_distance_between("a1234a", 'a', 5);
	test_distance_between("a1234", 'a', -1);
	test_distance_between("123456a12334a123a", 'a', 6);
	test_distance_between("", 'a', -1);


	test_num = 1;
	printf("\nTesting string_between()\n");
	test_string_between("a1234a", 'a', "1234");
	test_string_between("a123241a34a", 'a', "123241");
	test_string_between("a1234", 'a', NULL);
	test_string_between("adette er svaretaasd2qd3asd12", 'a', "dette er sv");
	test_string_between("", 'a', NULL);


	test_num = 1;
	printf("\nTesting stringsum2()\n");
	test_stringsum2("abcd", 10);
	test_stringsum2("abcd!", -1);
	test_stringsum2("bbbdbbbbbdbbdbbbbbddbbbbbdbbdbbbbdbd", 90);
	test_stringsum2("", 0);

	test_num = 1;
	printf("\nTesting split()\n");
	test_split("abcd", (char *[]){ "abcd", NULL });
	test_split("Hei du", (char *[]){ "Hei", "du", NULL });
	test_split("Dette er mange ord", (char *[]){ "Dette", "er", "mange", "ord", NULL });
	test_split("Dette er helt utrolig mange ord ingen har sett saa mange ord foer", (char *[]){ "Dette", "er", "helt", "utrolig", "mange", "ord", "ingen", "har", "sett", "saa", "mange", "ord", "foer", NULL });

	return 0;
}
