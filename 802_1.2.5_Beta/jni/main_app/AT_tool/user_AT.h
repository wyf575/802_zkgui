#ifndef USER_AT_H
#define USER_AT_H

int send_recv_AT_cmd(const char *cmd, char *need_ret, char *recv_buf, int buf_len);


#endif // !USER_AT_H