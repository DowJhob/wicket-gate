#ifndef SETTINGS_H
#define SETTINGS_H

struct connParam
{
    int readTimeout;
    int maxConnectionHandlers;
    int maxIdleHandlers;
    int maxSize;
    int maxMultiPartSize;
};

#endif // SETTINGS_H
