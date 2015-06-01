#pragma once

///////////////
/// Node part
///////////////

template <typename T>
std::size_t
Node<T>::Size() const
{
    return 1 + (Left ? Left->Size() : 0) + (Right ? Right->Size() : 0);
}

template <typename T>
void
Node<T>::PrettyPrint(std::size_t p_Depth)
{
    for (int i = 0; i < p_Depth; i++)
        std::cout << "\t";

    std::cout << "{" << Key << ", ";
    if (Left != nullptr)
    {
        std::cout << std::endl;
        Left->PrettyPrint(p_Depth + 1);
    }
    else
        std::cout << "{}";
    std::cout << ", ";

    if (Right != nullptr)
    {
        std::cout << std::endl;
        Right->PrettyPrint(p_Depth + 1);
    }
    else
        std::cout << "{}";
    std::cout << "}";
}

///////////////
/// SGT part
///////////////

template <typename T,
          typename Alloc,
          typename PtrAlloc>
SPG<T, Alloc, PtrAlloc>::SPG(float p_Alpha)
    :
        m_Alpha(p_Alpha),
        m_Size(0)
{
    /// We assert that the alpha factor given is right.
    assert(m_Alpha >= 0.5f && m_Alpha <= 1.0f);
}

template <typename T,
          typename Alloc,
          typename PtrAlloc>
SPG<T, Alloc, PtrAlloc>::~SPG()
{
    if (m_Impl.m_Root)
    {
        DestroyRec(m_Impl.m_Root);
        DestroyNode(m_Impl.m_Root);
    }
}

template <typename T,
          typename Alloc,
          typename PtrAlloc>
std::size_t
SPG<T, Alloc, PtrAlloc>::Size() const
{
    return m_Size;
}

template <typename T,
          typename Alloc,
          typename PtrAlloc>
typename SPG<T, Alloc, PtrAlloc>::link_type
SPG<T, Alloc, PtrAlloc>::find(value_type const& p_Key)
{
    return InternalFind(m_Impl.m_Root, p_Key);
}

template <typename T,
          typename Alloc,
          typename PtrAlloc>
bool
SPG<T, Alloc, PtrAlloc>::insert(value_type const& p_Key)
{
    /// If the tree has no elements, we put the new node as root.
    if (!m_Impl.m_Root)
    {
        m_Impl.m_Root = CreateNode(p_Key);
        m_Impl.m_Root->Left = nullptr;
        m_Impl.m_Root->Right = nullptr;
        ++m_Size;
        return true;
    }

    /// We allocate the array of the parents. Size is the maximum height of the tree.
    std::size_t l_Size = 900;//static_cast<std::size_t>(HeightAlpha(m_Size)) + 2;
    link_type* l_Parts = AllocatePointers(l_Size);

    auto&& l_Result = InsertKey(m_Impl.m_Root, p_Key, l_Parts);
    auto&& l_Height = std::get<0>(l_Result);
    auto&& l_NewNode = std::get<1>(l_Result);

    ++m_Size;

    if (l_Height == -1)
        return false;
    /// If the height is greater than the alpha height, we rebalance the tree.
    else if (false && l_Height > HeightAlpha(m_Size))
    {
        /// We find the node that is making the unbalance and rebuild the sub-tree.
        auto&& l_Result = FindScapeGoatNode(l_NewNode, l_Parts, l_Height - 1);
        auto l_ScapeGoatNode = std::get<0>(l_Result);
        auto l_ParentSG = std::get<1>(l_Result);
        l_ScapeGoatNode = RebuildTree(l_ScapeGoatNode->Size(), l_ScapeGoatNode);

        /// We link back the new subtree to the current tree.
        if (l_ParentSG)
        {
            if (l_ScapeGoatNode->Key <= l_ParentSG->Key)
                l_ParentSG->Left = l_ScapeGoatNode;
            else
                l_ParentSG->Right = l_ScapeGoatNode;
        }
        else
            m_Impl.m_Root = l_ScapeGoatNode;
    }

    /// Deallocate the parents stack.
    DeallocatePointers(l_Parts, l_Size);

    return true;
}

template <typename T,
          typename Alloc,
          typename PtrAlloc>
void
SPG<T, Alloc, PtrAlloc>::PrettyPrint() const
{
    m_Impl.m_Root->PrettyPrint(0);
    std::cout << std::endl;
}

template <typename T,
          typename Alloc,
          typename PtrAlloc>
std::pair<typename SPG<T, Alloc, PtrAlloc>::link_type, typename SPG<T, Alloc, PtrAlloc>::link_type>
SPG<T, Alloc, PtrAlloc>::FindScapeGoatNode(link_type p_Node, link_type* p_Parents, int p_Ind) const
{
    assert(p_Node != nullptr);

    link_type l_Parent = nullptr;
    link_type l_Sibling = nullptr;

    std::size_t l_Size = 1;
    std::size_t l_TotalSize = 0;
    std::size_t l_Height = 0;

    std::stack<std::pair<link_type, link_type>> l_SGNodes;

    while (p_Ind >= 0)
    {
        l_Parent = p_Parents[p_Ind--];
        l_Height++;

        assert(l_Parent);

        /// We only recalculate the sibling subtree size.
        l_Sibling = p_Node->Key <= l_Parent->Key ? l_Parent->Right : l_Parent->Left;
        l_TotalSize = 1 + l_Size + (l_Sibling ? l_Sibling->Size() : 0);
        if (l_Height > HeightAlpha(l_TotalSize))
            l_SGNodes.push(std::make_pair(l_Parent, p_Ind < 0 ? nullptr : p_Parents[p_Ind]));

        /// If we get a deep enough ancestor, we stop and return.
        if (l_SGNodes.size() == 2)
            return l_SGNodes.top();

        p_Node = l_Parent;
        l_Size = l_TotalSize;
    }

    if (!l_SGNodes.empty())
        return l_SGNodes.top();

    return std::make_pair(m_Impl.m_Root, nullptr);
}

template <typename T,
          typename Alloc,
          typename PtrAlloc>
typename SPG<T, Alloc, PtrAlloc>::link_type
SPG<T, Alloc, PtrAlloc>::BuildHeightBalancedTree(link_type* p_Head, int p_Size)
{
    if (p_Size <= 0)
        return nullptr;

    /// Build left subtree.
    link_type l_Left = BuildHeightBalancedTree(p_Head, p_Size / 2);

    /// Build root from actual node.
    link_type l_Parent = *p_Head;

    /// We link parent and child.
    l_Parent->Left = l_Left;

    /// Advance the pointer to the next element.
    *p_Head = (*p_Head)->Right;

    /// Build the right subtree and link to parent.
    l_Parent->Right = BuildHeightBalancedTree(p_Head, p_Size - p_Size / 2 - 1);

    return l_Parent;
}

template <typename T,
          typename Alloc,
          typename PtrAlloc>
typename SPG<T, Alloc, PtrAlloc>::link_type
SPG<T, Alloc, PtrAlloc>::RebuildTree(std::size_t p_Size, link_type p_Root)
{
    link_type l_Head = FlattenTree(p_Root, nullptr);
    return BuildHeightBalancedTree(&l_Head, p_Size);
}

template <typename T,
          typename Alloc,
          typename PtrAlloc>
typename SPG<T, Alloc, PtrAlloc>::link_type
SPG<T, Alloc, PtrAlloc>::FlattenTree(link_type p_Root, link_type p_Head)
{
    if (p_Root == nullptr)
        return p_Head;

    p_Root->Right = FlattenTree(p_Root->Right, p_Head);
    link_type l_Tmp = p_Root->Left;
    p_Root->Left = nullptr;
    return FlattenTree(l_Tmp, p_Root);
}

template <typename T,
          typename Alloc,
          typename PtrAlloc>
std::pair<int, typename SPG<T, Alloc, PtrAlloc>::link_type>
SPG<T, Alloc, PtrAlloc>::InsertKey(link_type p_Root, int p_Key, link_type* p_Parents)
{
    std::size_t l_Height = 0;

    while (p_Root)
    {
        /// We save the parents.
        p_Parents[l_Height] = p_Root;

        /// If the given key already exists, we return a negative height.
        if (p_Root->Key == p_Key)
            return std::make_pair(-1, nullptr);
        /// We look for a left/right place to put our new node.
        else if (p_Key <= p_Root->Key && p_Root->Left)
            p_Root = p_Root->Left;
        else if (p_Root->Right && p_Key > p_Root->Key)
            p_Root = p_Root->Right;
        else
            break;

        l_Height++;
    }

    /// Build our new node.
    auto l_NewNode = CreateNode(p_Key);
    l_NewNode->Left = nullptr;
    l_NewNode->Right = nullptr;

    /// We link ourself with the parent.
    if (l_NewNode->Key <= p_Root->Key)
        p_Root->Left = l_NewNode;
    else
        p_Root->Right = l_NewNode;

    return std::make_pair(l_Height + 1, l_NewNode);
}

template <typename T,
          typename Alloc,
          typename PtrAlloc>
void
SPG<T, Alloc, PtrAlloc>::DestroyRec(link_type p_N)
{
    if (p_N->Left)
    {
        DestroyRec(p_N->Left);
        DestroyNode(p_N->Left);
    }

    if (p_N->Right)
    {
        DestroyRec(p_N->Right);
        DestroyNode(p_N->Right);
    }
}

template <typename T,
          typename Alloc,
          typename PtrAlloc>
typename SPG<T, Alloc, PtrAlloc>::link_type
SPG<T, Alloc, PtrAlloc>::InternalFind(link_type p_Node, int p_Key)
{
    if (p_Node == nullptr || p_Node->Key == p_Key)
        return p_Node;
    else if (p_Key <= p_Node->Key)
        return InternalFind(p_Node->Left, p_Key);
    else
        return InternalFind(p_Node->Right, p_Key);
}

/////////////////////////////////////////////////
///// Non recursive rebuilding method.
///// Implementation from full paper version.
/////////////////////////////////////////////////

template <typename T>
struct BuildingElement
{
    Node<T>* Node;
    std::size_t Height;
    bool LacksRightSon;
    bool LacksFather;
};

template <typename T,
          typename Alloc,
          typename PtrAlloc>
typename SPG<T, Alloc, PtrAlloc>::link_type
SPG<T, Alloc, PtrAlloc>::RebuildTree2(std::size_t p_N, link_type p_SPN)
{
    int l_InsertType = 0; /// 0 is left, 1 is right and 2 is center.
    int l_SlotsInLastLevel = std::pow(2, std::floor(std::log(p_N)));
    int l_NodesForLastLevel = p_N - l_SlotsInLastLevel + 1;
    float l_Ratio = float(l_NodesForLastLevel) / float(l_SlotsInLastLevel);
    std::size_t l_MaxDepth = HeightAlpha(p_N) + 2;
    auto l_Ind = 0;

    std::stack<link_type> l_RuiningStack;
    BuildingElement<T>* l_BuildingStack = new BuildingElement<T>[l_MaxDepth];

    /// Definition of the GetNextNode function.
    auto GetNextNode = [&l_RuiningStack]() -> link_type
    {
        auto l_NextNode = l_RuiningStack.top();
        decltype(l_NextNode) l_FatherNode = nullptr;

        while (l_NextNode->Left != nullptr)
        {
            l_FatherNode = l_NextNode;
            l_NextNode = l_NextNode->Left;
        }

        if (l_NextNode == l_RuiningStack.top())
            l_RuiningStack.pop();
        else
            l_FatherNode = nullptr;

        if (l_NextNode->Right != nullptr)
            l_RuiningStack.push(l_NextNode->Right);

        return l_NextNode;
    };

    auto AddNonLeaf = [l_BuildingStack, &l_Ind](link_type p_NextNode)
    {
        p_NextNode->Left = l_BuildingStack[l_Ind].Node;
        auto l_NextNodeHeight = l_BuildingStack[l_Ind--].Height + 1;

        if (l_Ind > 1 && l_BuildingStack[l_Ind - 1].Height == l_NextNodeHeight + 1)
        {
            l_BuildingStack[l_Ind].Node->Right = p_NextNode;
            if (!l_BuildingStack[l_Ind].LacksFather)
                l_Ind--;
            else
                l_BuildingStack[l_Ind].LacksRightSon = false;

            l_BuildingStack[++l_Ind] = {p_NextNode, l_NextNodeHeight, true, false};
        }
        else
            l_BuildingStack[++l_Ind] = {p_NextNode, l_NextNodeHeight, true, true};

        if (l_BuildingStack[l_Ind].Height > 1)
            return 0;
        else
            return 1;
    };

    auto SkipALeaf = [AddNonLeaf, l_BuildingStack, &l_Ind](link_type p_NextNode, int p_InsertType)
    {
        if (p_InsertType == 0)
        {
            p_NextNode->Left = nullptr;
            if (l_BuildingStack[l_Ind].Height == 2)
            {
                l_BuildingStack[l_Ind].Node->Right = p_NextNode;
                if (!l_BuildingStack[l_Ind].LacksFather)
                    l_Ind--;
                else
                    l_BuildingStack[l_Ind].LacksRightSon = false;
                l_BuildingStack[++l_Ind] = {p_NextNode, 1, true, false};
            }
            else
                l_BuildingStack[++l_Ind] = {p_NextNode, 1, true, true};

            return 1;
        }
        else
        {
            /// Skip a right leaf.
            l_BuildingStack[l_Ind].Node->Right = nullptr;
            if (!l_BuildingStack[l_Ind].LacksFather)
                l_Ind--;
            else
                l_BuildingStack[l_Ind].LacksRightSon = false;

            return AddNonLeaf(p_NextNode);
        }
    };

    auto AddALeaf = [l_BuildingStack, &l_Ind](link_type p_NextNode, int p_InsertType)
    {
        p_NextNode->Right = nullptr;
        p_NextNode->Left = nullptr;

        if (p_InsertType == 0)
            l_BuildingStack[++l_Ind] = {p_NextNode, 0, false, true};
        else
        {
            l_BuildingStack[l_Ind].Node->Right = p_NextNode;
            if (l_BuildingStack[l_Ind].LacksFather)
                l_BuildingStack[l_Ind].LacksRightSon = false;
            else
                l_Ind--;
        }

        return 2;
    };

    /// Definition of AddNewNode function.
    auto AddNewNode = [&SkipALeaf, &AddALeaf, &AddNonLeaf, &l_NodesForLastLevel, &l_SlotsInLastLevel, &l_Ratio](link_type p_NextNode, int p_InsertType)
    {
        if (p_InsertType != 2)
        {
            l_SlotsInLastLevel--;
            if (l_SlotsInLastLevel > 0 && float(l_NodesForLastLevel) / float(l_SlotsInLastLevel) < l_Ratio)
                return SkipALeaf(p_NextNode, p_InsertType);

            l_NodesForLastLevel--;
            return AddALeaf(p_NextNode, p_InsertType);
        }
        else
            return AddNonLeaf(p_NextNode);
    };

    l_RuiningStack.push(p_SPN);

    for (int i = 0; i < p_N; i++)
        l_InsertType = AddNewNode(GetNextNode(), l_InsertType);

    return l_BuildingStack[l_Ind].Node;
}
