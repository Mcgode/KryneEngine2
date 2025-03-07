/**
 * @file
 * @author Max Godefroy
 * @date 14/11/2024.
 */

#pragma once

#define KE_MODULES_RENDER_GRAPH_DECLARATION_BUILDER_IMPL(builderName, dataType, ownerType) \
public:                                                                                    \
    builderName(dataType& _item, ownerType* _owner)                                        \
        : m_item(_item)                                                                    \
        , m_owner(_owner)                                                                  \
    {}                                                                                     \
                                                                                           \
    ownerType& Done() { return *m_owner; }                                                 \
                                                                                           \
    dataType& GetItem() { return m_item; }                                                 \
                                                                                           \
private:                                                                                   \
    dataType& m_item;                                                                      \
    ownerType* m_owner
