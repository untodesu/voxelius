#ifndef F3A2B6C1_9D4E_4A7B_8C3F_2E1D5A6B7C8D
#define F3A2B6C1_9D4E_4A7B_8C3F_2E1D5A6B7C8D

class Camera;

namespace head
{
void init(void);
void init_late(void);
void shutdown(void);
} // namespace head

namespace head
{
bool prepare(void);
void present(void);
} // namespace head

namespace head
{
void render(void);
} // namespace head

#endif /* F3A2B6C1_9D4E_4A7B_8C3F_2E1D5A6B7C8D */
