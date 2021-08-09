#include<dwnld.h>

int main(int argc, char **argv)
{
	if (argc <= 1) {
		fprintf(stderr, "dwnld: not enough arguments\n");
		return 1;
	}
	else if (argc >= 3) {
		fprintf(stderr, "dwnld: too many arguments\n");
		return 1;
	}

	dwnld(argv[1], "80");

	return 0;
}
