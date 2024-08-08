
#if defined(_spriv_)
#   define vkPushConstant [[vk::push_constant]]
#   define vkLocation(x) [[vk::location(x)]]
#   define vkBinding(x, y) [[vk::binding(x, y)]]
#else
#   define vkPushConstant
#   define vkLocation(x)
#   define vkBinding(x, y)
#endif