import lldb

def __lldb_init_module(debugger, internal_dict):
    def addSummary(type_name, summary_name, templated=True):
        command = None
        if templated:
            command = f'type summary add -x "^{type_name}" -F {__name__}.{summary_name} -w KryneEngine'
        else:
            command = f'type synthetic add {type_name} -F {__name__}.{summary_name} -w KryneEngine'
        debugger.HandleCommand(command)

    def addSytheticChildrenProvider(type_name, provider_name, templated=True):
        command = None
        if templated:
            command = f'type synthetic add -x "^{type_name}" --python-class {__name__}.{provider_name} -w KryneEngine'
        else:
            command = f'type synthetic add {type_name} --python-class {__name__}.{provider_name} -w KryneEngine'
        debugger.HandleCommand(command)

    addSummary("KryneEngine::DynamicArray<.*>", dynamic_array_summary.__name__)
    addSytheticChildrenProvider("KryneEngine::DynamicArray<.*>", DynamicArrayChildrenProvider.__name__)
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