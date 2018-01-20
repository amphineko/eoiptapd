#ifndef TAP_H_
#define TAP_H_

void tap_listen(struct environment_t *env);

void tap_send(struct environment_t *env, const char *frame, size_t frame_size);

#endif
