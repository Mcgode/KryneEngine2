import lldb

def __lldb_init_module(debugger, internal_dict):
    def add_summary(type_name, summary_name, templated=True):
        command = None
        if templated:
            command = f'type summary add -x "^{type_name}" -F {__name__}.{summary_name} -w KryneEngine'
        else:
            command = f'type synthetic add {type_name} -F {__name__}.{summary_name} -w KryneEngine'
        debugger.HandleCommand(command)

    def add_synthetic_children_provider(type_name, provider_name, templated=True):
        command = None
        if templated:
            command = f'type synthetic add -x "^{type_name}" --python-class {__name__}.{provider_name} -w KryneEngine'
        else:
            command = f'type synthetic add {type_name} --python-class {__name__}.{provider_name} -w KryneEngine'
        debugger.HandleCommand(command)

    add_summary("KryneEngine::DynamicArray<.*>", dynamic_array_summary.__name__)
    add_synthetic_children_provider("KryneEngine::DynamicArray<.*>", DynamicArrayChildrenProvider.__name__)

    add_summary("KryneEngine::Math::Vector2Base<.*>", vector_base_summary.__name__)
    add_summary("KryneEngine::Math::Vector3Base<.*>", vector_base_summary.__name__)
    add_summary("KryneEngine::Math::Vector4Base<.*>", vector_base_summary.__name__)
    for type in ["float", "double", "int", "uint"]:
        for count in [2, 3, 4]:
            add_summary(f"KryneEngine::{type}{count}", vector_base_summary.__name__)
            add_summary(f"KryneEngine::{type}{count}_simd", vector_base_summary.__name__)

    debugger.HandleCommand("type category enable KryneEngine")

def dynamic_array_summary(value_object, internal_dict):
    return f'size={value_object.GetChildMemberWithName("[size]").GetValueAsSigned()}'

class DynamicArrayChildrenProvider:
    def __init__(self, value_object, internal_dict):
        self.value_object = value_object
        self.count = 0
        self.size = None
        self.data_type = None
        self.data_size = None
        self.data_ptr = None
        self.update()

    def num_children(self):
        return self.count + 1

    def get_child_index(self, name: str):
        try:
            return 1 + int(name.lstrip('[').rstrip(']'))
        except:
            return 0 if name == "[size]" else -1

    def get_child_at_index(self, index):
        if index == 0:
            return self.size.Clone("[size]")
        elif index - 1 < self.count:
            offset = self.data_size * (index - 1)
            return self.data_ptr.CreateChildAtOffset(f'[{index - 1}]', offset, self.data_type)
        return None

    def update(self):
        self.size = self.value_object.GetChildMemberWithName("m_count")
        self.count = self.size.GetValueAsSigned()

        self.data_type = self.value_object.GetType().GetTemplateArgumentType(0)
        self.data_size = self.data_type.GetByteSize()
        self.data_ptr = self.value_object.GetChildMemberWithName("m_array")

    def has_children(self):
        return self.count > 0

def vector_base_summary(value_object, internal_dict):
    x = value_object.GetChildMemberWithName("x").GetValue()
    y = value_object.GetChildMemberWithName("y").GetValue()
    if value_object.GetChildMemberWithName("z").IsValid():
        z = value_object.GetChildMemberWithName("z").GetValue()
        if value_object.GetChildMemberWithName("w").IsValid():
            w = value_object.GetChildMemberWithName("w").GetValue()
            return f'({x}, {y}, {z}, {w})'
        return f'({x}, {y}, {z})'
    return f'({x}, {y})'
