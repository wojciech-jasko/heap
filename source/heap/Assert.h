#ifndef ASSERT_H
#define ASSERT_H

#define ASSERT(_cond)  \
    do                 \
    {                  \
        if (!(_cond))  \
        {              \
            _assert(); \
        }              \
    } while (0)

void _assert(void);

#endif /* #ifndef ASSERT_H */
