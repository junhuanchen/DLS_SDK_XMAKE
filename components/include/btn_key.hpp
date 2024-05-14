
#include "dls_lib.hpp"

struct btn_key
{
    int input_event0;
    fd_set readfd;
    struct timeval timeout;
};

int btn_key_load(struct btn_key *self)
{
    self->input_event0 = 0;
    
    FD_ZERO(&self->readfd);

    self->input_event0 = open("/dev/input/event0", O_RDONLY | O_NONBLOCK);
    if (self->input_event0 <= 0)
    {
        perror("open /dev/input/event0 device error!\n");
        abort();
    }

    self->timeout.tv_sec = 0;
    self->timeout.tv_usec = 40;

    return 0;
}

int btn_key_loop(struct btn_key *self, uint16_t *code, uint16_t *value)
{

    int ret = 0;
    FD_SET(self->input_event0, &self->readfd);
    ret = select(self->input_event0 + 1, &self->readfd, NULL, NULL, &self->timeout);
    if (ret != -1 && FD_ISSET(self->input_event0, &self->readfd))
    {
        struct input_event event;
        if (read(self->input_event0, &event, sizeof(event)) == sizeof(event))
        {
            if ((event.type == EV_KEY) && (event.value == 0 || event.value == 1))
            {
                *code = event.code;
                *value = event.value;
                return 1;
            }
        }
    }
    return 0;
}

int btn_key_free(struct btn_key *self)
{
    if (self->input_event0 > 0) close(self->input_event0);
    return 0;
}
