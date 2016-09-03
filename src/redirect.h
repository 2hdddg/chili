#pragma once

#define CHILI_REDIRECT_MAX_PATH 255
#define CHILI_REDIRECT_MAX_NAME 255

/* @brief Initializes redirection.
 * @param enable Flag to enable/disable, checked by other
 *        functions in module.
 * @param path Directory where redirection files
 *        will be placed, created if it doesnt exist
 *
 * @return Negative on error, positive on success.
 * @return Void
*/
int chili_redirect_begin(int enable, const char *path);

/* @bref Redirects stdout to a file.
 *
 * If module is disabled this will do nothing.
 *
 * @param name Name of file that will contain stdout
 * @return Void
*/
void chili_redirect_start(const char *name);

/* @brief Restores stdout to it's former glory.
 * @return Void
*/
void chili_redirect_stop();

/* @brief Prints a previously redirected stdout session
 *        to stdout.
 *
 * If module is disabled this will print nothing.
 * Redirection needs to be stopped prior to calling
 * this function.
 *
 * @param name Name used to start redirection
 * @param before String to send to stdout before content,
 *        or null if nothing to print
 * @param after String to send to stdout after content,
          or null if nothing to print
 * @return Void
*/
void chili_redirect_print(const char *name, const char *before,
                          const char *after);

/* @brief Releases allocated resources
 * @return Void
*/
void chili_redirect_end();

