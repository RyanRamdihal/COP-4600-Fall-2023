/**
 * @file   test.c
 * @author Derek Molloy
 * @date   7 April 2015
 * @version 0.1
 * @brief  A Linux user space program that communicates with the charkmod.c LKM. It passes a
 * string to the LKM and reads the response from the LKM. For this example to work the device
 * must be called /dev/charkmod.
 * @see http://www.derekmolloy.ie/ for a full description and follow-up descriptions.
 *
 * Adapted for COP 4600 by Dr. John Aedo
 */
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>

#define BUFFER_LENGTH 256           ///< The buffer length (crude but fine)
static char receive[BUFFER_LENGTH]; ///< The receive buffer from the LKM
static char *read_path;
static char *write_path;

// Read/write 1000 times to move the index around as much as possible
int basicReadWrite1k() {
	int read_fd, write_fd, ret;
	char readBuf[1001];
	char intBuf[1000];

	// Fill up buffer with sequential bytes for testing purposes
	for (int i = 0; i <= 1000; i++) {
		intBuf[i] = 'a' + (i % 7); // mod by a prime for maximum testing potential
	}

	for (int i = 0; i < 1000; i++) {
		read_fd = open(read_path, O_RDWR);
        write_fd = open(write_path, O_RDWR);
		ret = write(write_fd, intBuf, 1000);
		if (ret < 0) goto fail; // Fail if write fails
		ret = read(read_fd, readBuf, 1001);
		if (ret < 0) goto fail; // Fail if read fails
		for (int i = 0; i < 1000; i++) {
			if (readBuf[i] != 'a' + (i % 7)) goto fail; // Fail if the value is not expected
		}
		// Check for null pointer
		if (readBuf[1000] != '\0') goto fail;
		close(read_fd);
        close(write_fd);
	}

	return 1; // PASS

	fail:
		close(read_fd);
        close(write_fd);
		return 0;
}

// Ensure that the file can only be opened once at a time.

void test(char *name, int (*testFunction)()) {
	int res;

	printf("Testing %s... ", name);
	res = testFunction();
	if (res) {
		printf("PASS\n");
	} else {
		printf("FAIL\n");
	}
}

int main(int argc, char *argv[])
{
    if (argc != 3)
    {
        printf("Usage: test <path to read device> <path to write device>\n");
        exit(0);
    }
    char *devicepath = argv[1];
	read_path = argv[1];
    write_path = argv[2];

	// Automated Tests
	test("basicReadWrite1k", basicReadWrite1k);

    int ret, read_fd, write_fd;
    char stringToSend[BUFFER_LENGTH];
    strncpy(stringToSend, "", sizeof(stringToSend));
    printf("Starting device test code example...\n");
    read_fd = open(read_path, O_RDWR); // Open the device with read/write access
    write_fd = open(write_path, O_RDWR);
    if (read_fd < 0 || write_fd < 0)
    {
        perror("Failed to open the device...");
        return errno;
    }
    printf("Type in a short string to send to the kernel module:\n");
    scanf("%[^\n]%*c", stringToSend); // Read in a string (with spaces)
    printf("Writing message to the device [%s].\n", stringToSend);
    ret = write(write_fd, stringToSend, strlen(stringToSend)); // Send the string to the LKM
    if (ret < 0)
    {
        perror("Failed to write the message to the device.");
        return errno;
    }

    printf("Press ENTER to read back from the device...\n");
    getchar();

    printf("Reading from the device...\n");
    ret = read(read_fd, receive, BUFFER_LENGTH); // Read the response from the LKM
    if (ret < 0)
    {
        perror("Failed to read the message from the device.");
        return errno;
    }
    printf("The received message is: [%s]\n", receive);
    printf("End of the program\n");
    return 0;
}
