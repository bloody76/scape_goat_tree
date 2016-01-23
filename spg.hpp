#pragma once
#include <cmath>
#include <iostream>
#include <cassert>
#include <stack>

/// Basic node structure for the scapegoat tree.
/// The advantage of this structure is that we only need
/// the left and right children of the node.
struct NodeBase
{
    NodeBase*   Left;
    NodeBase*   Right;

    /// print the current subtree.
    /// @p_Depth is the number of tabs that
    /// will be print.
    template <typename T>
    void print(std::size_t p_Depth) const;
};

/// This is the templated structure that will contain the key.
template <typename T>
struct Node : public NodeBase
{
    T Key;
};

template <typename T>
class spg_reverse_iterator
{
    public:
        using self_type = spg_reverse_iterator<T>;
        using value_type = T;
        using iterator = spg_reverse_iterator<T>;
        using reference = value_type&;
        using pointer = value_type&;
        using link_type = Node<value_type>*;

        link_type m_Node;
        std::stack<link_type> m_Parents;

        spg_reverse_iterator()
            : m_Node(nullptr),
            m_Parents()
        {

        }

        spg_reverse_iterator(link_type p_Node)
            : m_Node(p_Node),
            m_Parents()
        {
            if (m_Node)
                go_right();
        }

        spg_reverse_iterator(self_type const& p_Itr)
        {
            if (this != &p_Itr)
            {
                m_Node = p_Itr.m_Node;
                m_Parents = p_Itr.m_Parents;
            }
        }

        reference operator*() const
        {
            return m_Node->Key;
        }

        pointer operator->() const
        {
            return &(operator*());
        }

        self_type& operator++()
        {
            incr();
            return *this;
        }

        self_type operator++(int)
        {
            auto l_Tmp = *this;
            incr();
            return l_Tmp;
        }

        self_type& operator--()
        {
            decr();
            return *this;
        }

        self_type operator--(int)
        {
            self_type l_Tmp = *this;
            decr();
            return l_Tmp;
        }

        bool operator==(self_type const& p_Rhs) const
        {
            return m_Node == p_Rhs.m_Node;
        }

        bool operator!=(self_type const& p_Rhs) const
        {
            return !(operator==(p_Rhs));
        }

    private:

        void incr()
        {
            if (!m_Node)
            {
                m_Node = nullptr;
                return;
            }

            m_Parents.pop();

            if (m_Node->Left)
            {
                m_Node = (link_type)m_Node->Left;
                go_right();
            }
            else if (!m_Parents.empty())
                m_Node = m_Parents.top();
            else
                m_Node = nullptr;
        }

        // Iterates on the right side of the tree as much as possible.
        void go_right()
        {
            while (m_Node->Right)
            {
                m_Parents.push(m_Node);
                m_Node = (link_type)m_Node->Right;
            }

            m_Parents.push(m_Node);
        }

        void decr()
        {
            if (!m_Node)
                return;

            if (m_Node->Right)
                m_Parents.push(m_Node->Right);
            m_Node = (link_type)m_Node->Right;
        }
};

template <typename T>
class spg_const_reverse_iterator
{
    public:
        using self_type = spg_const_reverse_iterator<T>;
        using value_type = T;
        using iterator = spg_reverse_iterator<T>;
        using const_iterator = spg_const_reverse_iterator<T>;
        using reference = value_type const&;
        using pointer = value_type const*;
        using link_type = Node<value_type> const*;

        link_type m_Node;
        std::stack<link_type> m_Parents;

        spg_const_reverse_iterator()
            : m_Node(nullptr),
            m_Parents()
        {

        }

        spg_const_reverse_iterator(link_type p_Node)
            : m_Node(p_Node),
            m_Parents()
        {
            if (m_Node)
                go_right();
        }

        spg_const_reverse_iterator(self_type const& p_Itr)
        {
            if (this != &p_Itr)
            {
                m_Node = p_Itr.m_Node;
                m_Parents = p_Itr.m_Parents;
            }
        }

        spg_const_reverse_iterator(iterator const& p_Itr)
        {
            if (this != &p_Itr)
            {
                m_Node = p_Itr.m_Node;
                m_Parents = p_Itr.m_Parents;
            }
        }

        reference operator*() const
        {
            return m_Node->Key;
        }

        pointer operator->() const
        {
            return &(operator*());
        }

        self_type& operator++()
        {
            incr();
            return *this;
        }

        self_type operator++(int)
        {
            auto l_Tmp = *this;
            incr();
            return l_Tmp;
        }

        self_type& operator--()
        {
            decr();
            return *this;
        }

        self_type operator--(int)
        {
            self_type l_Tmp = *this;
            decr();
            return l_Tmp;
        }

        bool operator==(self_type const& p_Rhs) const
        {
            return m_Node == p_Rhs.m_Node;
        }

        bool operator!=(self_type const& p_Rhs) const
        {
            return !(operator==(p_Rhs));
        }

    private:

        void incr()
        {
            if (!m_Node)
            {
                m_Node = nullptr;
                return;
            }

            m_Parents.pop();

            if (m_Node->Left)
            {
                m_Node = (link_type)m_Node->Left;
                go_right();
            }
            else if (!m_Parents.empty())
                m_Node = m_Parents.top();
            else
                m_Node = nullptr;
        }

        // Iterates on the right side of the tree as much as possible.
        void go_right()
        {
            while (m_Node->Right)
            {
                m_Parents.push(m_Node);
                m_Node = (link_type)m_Node->Right;
            }

            m_Parents.push(m_Node);
        }

        void decr()
        {
            if (!m_Node)
                return;

            if (m_Node->Right)
                m_Parents.push(m_Node->Right);
            m_Node = (link_type)m_Node->Right;
        }
};

template <typename T>
class spg_iterator
{
    public:
        using self_type = spg_iterator<T>;
        using value_type = T;
        using iterator = spg_iterator<T>;
        using reference = value_type&;
        using pointer = value_type&;
        using link_type = Node<value_type>*;

        link_type m_Node;
        std::stack<link_type> m_Parents;

        spg_iterator()
            : m_Node(nullptr),
            m_Parents()
        {

        }

        spg_iterator(link_type p_Node)
            : m_Node(p_Node),
            m_Parents()
        {
            if (m_Node)
                go_left();
        }

        spg_iterator(self_type const& p_Itr)
        {
            if (this != &p_Itr)
            {
                m_Node = p_Itr.m_Node;
                m_Parents = p_Itr.m_Parents;
            }
        }

        reference operator*() const
        {
            return m_Node->Key;
        }

        pointer operator->() const
        {
            return &(operator*());
        }

        self_type& operator++()
        {
            incr();
            return *this;
        }

        self_type operator++(int)
        {
            auto l_Tmp = *this;
            incr();
            return l_Tmp;
        }

        self_type& operator--()
        {
            decr();
            return *this;
        }

        self_type operator--(int)
        {
            self_type l_Tmp = *this;
            decr();
            return l_Tmp;
        }

        bool operator==(self_type const& p_Rhs) const
        {
            return m_Node == p_Rhs.m_Node;
        }

        bool operator!=(self_type const& p_Rhs) const
        {
            return !(operator==(p_Rhs));
        }

    private:

        void incr()
        {
            if (!m_Node)
            {
                m_Node = nullptr;
                return;
            }

            m_Parents.pop();

            if (m_Node->Right)
            {
                m_Node = (link_type)m_Node->Right;
                go_left();
            }
            else if (!m_Parents.empty())
                m_Node = m_Parents.top();
            else
                m_Node = nullptr;
        }

        // Iterates on the left side of the tree as much as possible.
        void go_left()
        {
            while (m_Node->Left)
            {
                m_Parents.push(m_Node);
                m_Node = (link_type)m_Node->Left;
            }

            m_Parents.push(m_Node);
        }

        void decr()
        {
            if (!m_Node)
                return;

            if (m_Node->Left)
                m_Parents.push(m_Node->Left);
            m_Node = (link_type)m_Node->Left;
        }
};

template <typename T>
class spg_const_iterator
{
    public:
        using self_type = spg_const_iterator<T>;
        using value_type = T;
        using iterator = spg_iterator<T>;
        using const_iterator = spg_const_iterator<T>;
        using reference = value_type const&;
        using pointer = value_type const&;
        using link_type = Node<value_type> const*;

        link_type m_Node;
        std::stack<link_type> m_Parents;

        spg_const_iterator()
            : m_Node(nullptr),
            m_Parents()
        {

        }

        spg_const_iterator(link_type p_Node)
            : m_Node(p_Node),
            m_Parents()
        {
            if (m_Node)
                go_left();
        }

        spg_const_iterator(self_type const& p_Itr)
        {
            if (this != &p_Itr)
            {
                m_Node = p_Itr.m_Node;
                m_Parents = p_Itr.m_Parents;
            }
        }

        spg_const_iterator(iterator const& p_Itr)
        {
            if (this != &p_Itr)
            {
                m_Node = p_Itr.m_Node;
                m_Parents = p_Itr.m_Parents;
            }
        }

        reference operator*() const
        {
            return m_Node->Key;
        }

        pointer operator->() const
        {
            return &(operator*());
        }

        self_type& operator++()
        {
            incr();
            return *this;
        }

        self_type operator++(int)
        {
            auto l_Tmp = *this;
            incr();
            return l_Tmp;
        }

        self_type& operator--()
        {
            decr();
            return *this;
        }

        self_type operator--(int)
        {
            self_type l_Tmp = *this;
            decr();
            return l_Tmp;
        }

        bool operator==(self_type const& p_Rhs) const
        {
            return m_Node == p_Rhs.m_Node;
        }

        bool operator!=(self_type const& p_Rhs) const
        {
            return !(operator==(p_Rhs));
        }

    private:

        void incr()
        {
            if (!m_Node)
            {
                m_Node = nullptr;
                return;
            }

            m_Parents.pop();

            if (m_Node->Right)
            {
                m_Node = (link_type)m_Node->Right;
                go_left();
            }
            else if (!m_Parents.empty())
                m_Node = m_Parents.top();
            else
                m_Node = nullptr;
        }

        // Iterates on the left side of the tree as much as possible.
        void go_left()
        {
            while (m_Node->Left)
            {
                m_Parents.push(m_Node);
                m_Node = (link_type)m_Node->Left;
            }

            m_Parents.push(m_Node);
        }

        void decr()
        {
            if (!m_Node)
                return;

            if (m_Node->Left)
                m_Parents.push(m_Node->Left);
            m_Node = (link_type)m_Node->Left;
        }
};

/// ScapeGoat tree implementation from the paper ScapeGoat Tree
/// of Igal Galperin and Ronald L. Rivest. The rebalancing method
/// is the one of Day/Stout/Warren.
template <typename T,
          typename Comparator = std::less<T>,
          typename Alloc = std::allocator<T>>
class SPG
{
    using NodeAllocator = typename Alloc::template rebind<Node<T>>::other;

    using node_base_type = NodeBase;
    using link_base_type = node_base_type*;

    using node_type = Node<T>;
    using link_type = node_type*;

    using allocator_type = Alloc;
    using value_type = T;

    using iterator = spg_iterator<T>;
    using const_iterator = spg_const_iterator<T>;
    using reverse_iterator = spg_reverse_iterator<T>;
    using const_reverse_iterator = spg_const_reverse_iterator<T>;

    public:
        /// Constructs a space goat tree.
        /// @p_Alpha : unbalance factor of the tree, MUST be in the interval [0.5, 1.0].
        SPG(float p_Alpha);

        /// We delete the pointers when we destroy our data structure.
        ~SPG();

        /// Returns the size of the tree.
        std::size_t size() const { return m_Size; };

        /// Returns true if the tree is empty.
        bool empty() const { return size() == 0; }

        /// Returns the node with the given key in the tree.
        /// @p_Key : The key we look for.
        link_type find(value_type const& p_Key);

        /// Insert a new node in the tree with the corresponding given key.
        /// It will rebalance the tree if needed according to the unbalance factor.
        /// @p_Key : The key to insert.
        /// Returns true if the key was inserted, false otherwise.
        bool insert(value_type const& p_Key);

        /// Erases the elements which value is p_Key.
        /// @p_Key : The key to erase.
        /// Returns the number of elements erased.
        std::size_t erase(value_type const& p_Key);

        /// print the tree on the cout.
        void print() const;

        ////////////////////////
        ///     Iterators.
        ////////////////////////

        iterator begin()
        {
            return iterator((link_type)m_Impl.m_Root);
        }

        reverse_iterator rbegin()
        {
            return reverse_iterator((link_type)m_Impl.m_Root);
        }

        const_iterator cbegin() const
        {
            return const_iterator((link_type)m_Impl.m_Root);
        }

        const_reverse_iterator crbegin() const
        {
            return const_reverse_iterator((link_type)m_Impl.m_Root);
        }

        iterator end()
        {
            return iterator(nullptr);
        }

        reverse_iterator rend()
        {
            return reverse_iterator(nullptr);
        }

        const_iterator cend() const
        {
            return const_iterator(nullptr);
        }

        const_reverse_iterator crend() const
        {
            return const_reverse_iterator(nullptr);
        }

    protected:

        ////////////////////////
        /// Allocator related.
        ////////////////////////

        /// Returns the node allocator.
        inline NodeAllocator& GetNodeAllocator()
        {
            return *static_cast<NodeAllocator*>(&m_Impl);
        }

        /// Returns the node allocator.
        inline NodeAllocator const& GetNodeAllocator() const
        {
            return *static_cast<NodeAllocator const*>(&m_Impl);
        }

        /// Returns the value_type allocator.
        inline allocator_type GetAllocator() const
        {
            return allocator_type(GetNodeAllocator());
        }

        /// Allocates one node and returns the adress of the new memory space.
        inline link_type AllocateNode()
        {
            return m_Impl.NodeAllocator::allocate(1);
        }

        /// Deallocate one node from the given adress node.
        /// @p_Node : The adress of the node memory to deallocate.
        inline void DeallocateNode(link_type p_Node)
        {
            m_Impl.NodeAllocator::deallocate(p_Node, 1);
        }

        /// Creates a node, allocates and constructs the value in it.
        /// @p_Val : The value of the node.
        /// Returns the pointer of the new node.
        inline link_type CreateNode(value_type const& p_Val)
        {
            auto l_Tmp = AllocateNode();

            try
            {
                GetNodeAllocator().construct(&l_Tmp->Key, p_Val);
            }
            catch (...)
            {
                DeallocateNode(l_Tmp);
                /// We should throw the exception here.
            }

            return l_Tmp;
        }

        /// Destroys the node given.
        /// @p_Node : The adress of the node to destroy.
        void DestroyNode(link_type p_Node)
        {
            GetAllocator().destroy(&p_Node->Key);
            DeallocateNode(p_Node);
        }

        ////////////////////////
        ////////////////////////

        /// Implementation class of the ScapeGoat tree.
        /// Corresponds to the allocator also.
        struct SPG_Impl : public NodeAllocator
        {
            link_base_type m_Root; ///< Root of the tree.
            Comparator m_KeyComparator;

            SPG_Impl(NodeAllocator const& p_Allocator = NodeAllocator(),
                     Comparator const& p_Comparator = Comparator())
                : NodeAllocator(p_Allocator),
                m_Root(nullptr),
                m_KeyComparator(p_Comparator)
            {
            }
        };

    private:
        /// Returns the key of a NodeBase.
        /// @p_NodeBase : The node.
        inline value_type const& GetKey(link_base_type p_NodeBase) const
        {
            return static_cast<link_type>(p_NodeBase)->Key;
        }

        /// Finds the space goat node which is the one who got a height greater than
        /// the authorized alpha height.
        /// @p_Node : The node to look from.
        /// @p_Parents : The stacked parents of the node.
        /// @p_Ind : The pointer to the top of the stacked parents.
        /// @p_TotalSize : The size of the subtree under the scapegoat node.
        /// Returns the space goat node if found, the root otherwise, and its parent.
        inline std::pair<link_type, link_type> FindScapeGoatNode(link_type p_Node, link_type* p_Parents, std::size_t p_Ind, std::size_t& p_TotalSize) const;

        /// Insert the given key in the tree.
        /// @p_Root : The root of the tree to insert into.
        /// @p_Key : The given key to insert.
        /// @p_Parents : The stacked parents for more ocess.
        /// @p_NewNode : The new node that will be allocated and created in the method.
        /// Returns a pair containing the height of the new node and its pointer.
        inline int InsertKey(link_type p_Root, value_type const& p_Key, link_type* p_Parents) const;

        /// Recursively destroy the whole tree.
        /// @p_N : The root of the subtree to destroy.
        void DestroyRec(link_base_type p_N);

        /// Calculate the alpha height of the tree based on the size given.
        /// @p_N : The size of the tree.
        /// Returns the alpha height value.
        inline float HeightAlpha(std::size_t p_N) const
        {
            return std::log(p_N) / m_Alpha;
        }

        /// Returns the node with the given key in the tree.
        /// @p_Node : The node to begin with.
        /// @p_Key : The key we look for.
        link_type InternalFind(link_type p_Node, int p_Key);

        /// Creates a node and returns it.
        /// @p_Key : The key of the new node.
        /// @p_Parent : The parent of the new node.
        inline link_type BuildNode(value_type const& p_Key,
                                   link_type p_Parent)
        {
            /// Build our new node.
            auto l_NewNode = CreateNode(p_Key);
            l_NewNode->Left = nullptr;
            l_NewNode->Right = nullptr;

            /// We link ourself with the parent.
            if (m_Impl.m_KeyComparator(p_Key, p_Parent->Key))
                p_Parent->Left = l_NewNode;
            else
                p_Parent->Right = l_NewNode;

            return l_NewNode;
        }

        /// Creates the root.
        /// @p_Key : The key of the root.
        void BuildRootNode(value_type const& p_Key)
        {
            m_Impl.m_Root = CreateNode(p_Key);
            m_Impl.m_Root->Left = nullptr;
            m_Impl.m_Root->Right = nullptr;
            ++m_Size;
        }

    public:
        link_type RebuildTree(std::size_t, link_base_type);

        float       m_Alpha;    ///< Alpha factor of the tree, says how much it can be unbalanced.
        SPG_Impl    m_Impl;     ///< The implementation and allocator of the ScapeGoat tree.
        std::size_t m_Size;     ///< Size of the tree.
};

#include "sgt.hxx"
