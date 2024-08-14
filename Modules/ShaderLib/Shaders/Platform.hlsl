
#ifdef __spirv__
#   define vkPushConstant [[vk::push_constant]]
#   define vkLocation(index) [[vk::location(index)]]
#   define vkBinding(index, set) [[vk::binding(index, set)]]
#else
#   define vkPushConstant
#   define vkLocation(index)
#   define vkBinding(index, set)
#endif