/*
  gpcxx/tree/detail/basic_cursor.hpp

  Copyright 2013 Karsten Ahnert

  Distributed under the Boost Software License, Version 1.0.
  (See accompanying file LICENSE_1_0.txt or
  copy at http://www.boost.org/LICENSE_1_0.txt)
*/


#ifndef GPCXX_TREE_DETAIL_BASIC_CURSOR_HPP_DEFINED
#define GPCXX_TREE_DETAIL_BASIC_CURSOR_HPP_DEFINED

#include <gpcxx/tree/detail/node_helpers.hpp>
#include <gpcxx/tree/cursor_traits.hpp>

#include <boost/iterator/iterator_facade.hpp>
#include <boost/iterator/iterator_concepts.hpp>

#include <type_traits>


#include <iostream>
using namespace std;


namespace gpcxx {

template< typename T , size_t MaxArity , typename A > class basic_tree;

namespace detail {


template< typename Node >
class basic_node_cursor : public boost::iterator_facade<
    basic_node_cursor< Node > ,                           // Derived-Iterator
    typename node_value_getter< Node >::type ,            // Value
    boost::random_access_traversal_tag // ,               // Category
    // boost::use_default ,                               // Reference
    // boost::use_default 
    >
{
    
    friend class boost::iterator_core_access;
    template< typename U > friend class basic_node_cursor;
    template< typename T , size_t MaxArity , typename A > friend class basic_tree;

    
    typedef Node node_type;
    typedef node_type* node_pointer;
    typedef typename std::remove_const< Node >::type real_node_type;    
    
    typedef typename node_base_getter< Node >::type node_base;
    typedef node_base* node_base_pointer;
    typedef typename std::remove_const< node_base >::type real_node_base;
    typedef real_node_base* real_node_base_pointer;
    typedef real_node_base const* const_real_node_base_pointer;
    

    
    typedef boost::iterator_facade<
        basic_node_cursor< Node > ,                        // Derived-Iterator
        typename node_value_getter< Node >::type ,         // Value
        boost::random_access_traversal_tag ,               // Category
        typename node_value_getter< Node >::type& ,        // Reference
        ptrdiff_t
    > base_type;
    
    template< typename OtherNode >
    struct other_node_enabler : public std::enable_if< std::is_convertible< typename OtherNode::value_type* , typename base_type::value_type* >::value >
    {
    };
    
public:
    
    // requirements from container concept via cursor concept
    // typedef typename base_type::reference const_reference;
    typedef size_t size_type;    
    
    // requirements from cursor concept
    typedef basic_node_cursor< node_type > cursor;
    typedef basic_node_cursor< real_node_type const > const_cursor;

    // typedef ascending_random_access_cursor_tag type;
    
    basic_node_cursor( node_base_pointer node = nullptr , size_type pos = 0 )
    : m_node( node ) , m_pos( pos ) { }
    
    basic_node_cursor( basic_node_cursor const& ) = default;
    basic_node_cursor& operator=( basic_node_cursor const& ) = default;

//    template< typename OtherNode , typename Enabler = typename other_node_enabler< OtherNode >::type >
//     basic_node_cursor( basic_node_cursor< OtherNode > const& other )
//     : m_node( other.m_node ) , m_pos( other.m_pos ) {}
    
    // emtpy()
    // swap()
    // operator==
    // operator!=
    
    
     

    // a.begin() // cursor
    // a.end() // cursor
    // a.cbegin() const_cursor
    // a.cend() const_cursor
    // a.parity() size_type   (std::distance(b.begin(), a) if b is a's parent.)
    // a.parent() const_cursor / cursor
    
    cursor begin( void )
    {
        return cursor( m_node->children( m_pos ) , 0 );
    }
    
    const_cursor begin( void ) const
    {
        return cbegin();
    }
    
    const_cursor cbegin( void ) const
    {
        return const_cursor( m_node->children( m_pos ) , 0 );
    }
    
    cursor end( void )
    {
        return cursor( m_node->children( m_pos ) , this->size() );
    }
    
    const_cursor end( void ) const
    {
        return cend();
    }
    
    const_cursor cend( void ) const
    {
        return const_cursor( m_node->children( m_pos ) , this->size() );
    }
    
    cursor parent( void )
    {
        assert( ! is_root() );
        auto parent_node = m_node->parent();
        return cursor( parent_node , parent_node->child_index( m_node ) );
    }

    const_cursor parent( void ) const
    {
        assert( ! is_root() );
        auto parent_node = m_node->parent();
        return const_cursor( parent_node , parent_node->child_index( m_node ) );
    }


    cursor children( size_type i )
    {
        return cursor( m_node->children( m_pos ) , i );
    }
    
    const_cursor children( size_type i ) const
    {
        return const_cursor( m_node->children( m_pos ) , i );
    }
    
    
    
    size_type size( void ) const noexcept
    {
        return m_node->children( m_pos )->size();
    }
    
    size_type max_size( void ) const noexcept
    {
        return m_node->max_size();
    }
    
    size_type height( void ) const noexcept
    {
        // if( node() == nullptr ) return 0;
        
        size_type h = 0;
        for( const_cursor s = begin() ; s != end() ; ++s )
            h = std::max( h , s.height() );
        return 1 + h;

    }

    size_type level( void ) const noexcept
    {
        if( m_node == nullptr ) return 0;
        if( m_node->parent() == nullptr ) return 0;
        return 1 + parent().level();
    }
    
    bool is_root( void ) const noexcept
    {
        return ( ( m_node->parent() == 0 ) && ( m_pos == 0 ) );
    }
    
    bool is_shoot( void ) const noexcept
    {
        return ( ( m_node->parent() == 0 ) && ( m_pos == 1 ) );
    }

    
    
public:

    node_base_pointer parent_node( void ) noexcept
    {
        return m_node;
    }

    const node_base_pointer parent_node( void ) const noexcept
    {
        return m_node;
    }

    node_base_pointer node( void ) noexcept
    {
        return m_node->children( m_pos );
    }

    const node_base_pointer node( void ) const noexcept
    {
        return m_node->children( m_pos );
    }
    
    /// REMOVE LATER, ONLY FOR DEBUGGING
    size_type pos( void ) const noexcept
    {
        return m_pos;
    }


private:

    void increment( void )
    {
        ++m_pos;
    }
    
    void decrement( void )
    {
        --m_pos;
    }
    
    void advance( typename base_type::difference_type n )
    {
        m_pos += n;
    }
    
    typename base_type::difference_type distance_to( basic_node_cursor const& other ) const
    {
        // TODO: implement
    }

    bool equal( basic_node_cursor const& other) const
    {
        return ( other.m_node == m_node ) && ( other.m_pos == m_pos );
    }

    typename base_type::reference dereference() const
    {
        return **static_cast< node_pointer >( m_node->children( m_pos ) );
    }
    
    
    node_base_pointer m_node;
    size_type m_pos;
};

template< typename Node1 , typename Node2 >
bool cursor_comp( basic_node_cursor< Node1 > const& c1 , basic_node_cursor< Node2 > const& c2 )
{
    if( c1.size() != c2.size() ) return false;
    if( *c1 != *c2 ) return false;
    for( size_t i=0 ; i<c1.size() ; ++i )
    {
        if( !cursor_comp( c1.children(i) , c2.children(i) ) ) return false;
    }
    return true;
}


} // namespace detail

template< typename Node >
struct is_cursor< detail::basic_node_cursor< Node > > : public std::true_type { };



} // namespace gpcxx


#endif // GPCXX_TREE_DETAIL_BASIC_CURSOR_HPP_DEFINED