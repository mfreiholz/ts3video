#ifndef IMPL_H
#define IMPL_H

#ifdef __cplusplus
extern "C" {
#endif

  /**
   * Public C-API
   * @version 1.0
   */
  int runClient(const char *serverAddress, unsigned short serverPort, const char *username);

#ifdef __cplusplus
}
#endif

#endif