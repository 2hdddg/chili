#pragma once

#define CHILI_REDIRECT_MAX_PATH 255
#define CHILI_REDIRECT_MAX_NAME 255

/* Initializes redirection.
 * enable - flag to enable/disable, checked by other
 *          functions in module.
 * path   - path to directory where redirection files
 *          will be placed, created if it doesnt exist
 *
 * Returns neg on error, pos on success.
*/
int chili_redirect_begin(int enable, const char *path);

/* Redirects stdout to a file. If module is disabled
 * this will do nothing.
 *
 * name - name of file that will contain stdout
*/
void chili_redirect_start(const char *name);

/* Restores stdout to it's former glory.
*/
void chili_redirect_stop();

/* Prints a previously redirected stdout session
 * to stdout. If module is disabled this will print
 * nothing.
 * Redirection needs to be stopped prior to calling
 * this function.
 *
 * name - name used to start redirection
 * before - string to send to stdout before content,
 *          or null if nothing to print
 * after - string to send to stdout after content,
           or null if nothing to print
*/
void chili_redirect_print(const char *name, const char *before,
                          const char *after);

/* Releases allocated resources
*/
void chili_redirect_end();
