/*
  gp/ga/ga2.hpp

  Copyright 2013 Karsten Ahnert

  Distributed under the Boost Software License, Version 1.0.
  (See accompanying file LICENSE_1_0.txt or
  copy at http://www.boost.org/LICENSE_1_0.txt)
*/


#ifndef GP_GA_GA2_HPP_DEFINED
#define GP_GA_GA2_HPP_DEFINED

#include <gp/operator/fitness_prob.hpp>

#include <functional>
#include <cassert>


namespace gp {
    
    
template< typename Population , typename Fitness , typename Context , typename Rng >
class genetic_evolver2
{
public:
    
    typedef Population population_type;
    typedef typename population_type::value_type individual_type;
    typedef Fitness fitness_type;
    typedef Context context_type;
    typedef Rng rng_type;

    typedef std::function< double( individual_type& , const context_type& ) > fitness_function_type;
    typedef std::function< individual_type( population_type const& , fitness_type const& ) > mutation_type;
    typedef std::function< std::pair< individual_type , individual_type >( population_type const& , fitness_type const& ) > crossover_type;
    typedef std::function< individual_type( population_type const& , fitness_type const& ) > reproduction_type;

    genetic_evolver2( double elite_rate , double mutation_rate , double crossover_rate , double reproduction_rate , rng_type &rng )
        : m_elite_rate( elite_rate ) , m_mutation_rate( mutation_rate ) , m_crossover_rate( crossover_rate ) , m_reproduction_rate( reproduction_rate )
        , m_rng( rng )
        , m_fitness_function() , m_mutation_function() , m_crossover_function() , m_reproduction_function()
    { }

    void next_generation( population_type &pop , fitness_type &fitness , const context_type &c )
    {
        reproduce( pop , fitness );
        calc_fitness( pop , fitness , c );
    }


    fitness_function_type& fitness_function( void ) { return m_fitness_function; }
    mutation_type& mutation_function( void ) { return m_mutation_function; }
    crossover_type& crossover_function( void ) { return m_crossover_function; }
    reproduction_type& reproduction_function( void ) { return m_reproduction_function; }

    const fitness_function_type& fitness_function( void ) const { return m_fitness_function; }
    const mutation_type& mutation_function( void ) const { return m_mutation_function; }
    const crossover_type& crossover_function( void ) const { return m_crossover_function; }
    const reproduction_type& reproduction_function( void ) const { return m_reproduction_function; }
    

private:


    void reproduce( population_type &pop , fitness_type &fitness )
    {
        assert( pop.size() == fitness.size() );
        
        fitness_prob< fitness_type , rng_type > prob( fitness , m_rng );
        size_t n = pop.size();
 
        population_type new_pop;
 
        // elite
        size_t n_elite = size_t( double( n ) * m_elite_rate );
        for( size_t i=0 ; i<n_elite ; ++i )
        {
            size_t index = prob.indices()[i] ;
            new_pop.push_back( pop[ index ] );
        }
        
        
        std::discrete_distribution< int > dist( { m_mutation_rate , m_crossover_rate , m_reproduction_rate } );
        while( new_pop.size() < n )
        {
            int choice = dist( m_rng );
            switch( choice )
            {
                case 0 : // mutation
                    {
                        individual_type mutated_tree = m_mutation_function( pop , fitness );
                        new_pop.push_back( std::move( mutated_tree ) );
                    }
                    break;
                case 1 : // crossover
                    {
                        std::pair< individual_type , individual_type > trees = m_crossover_function( pop , fitness );
                        if( new_pop.size() == ( n - 1 ) )
                        {
                            new_pop.push_back( std::move( trees.first ) );
                        }
                        else
                        {
                            new_pop.push_back( std::move( trees.first ) );
                            new_pop.push_back( std::move( trees.second ) );
                        }
                    }
                    break;
                case 2 : // reproduction
                    {
                        individual_type reproduced_node = m_reproduction_function( pop , fitness );
                        new_pop.push_back( std::move( reproduced_node ) );
                    }
                    break;
            }
        }
        
        pop = std::move( new_pop );
    }


    void calc_fitness( population_type &pop , fitness_type &fitness , const context_type &c )
    {
        for( size_t i=0 ; i<pop.size() ; ++i )
            fitness[i] = m_fitness_function( pop[i] , c );
    }

    double m_elite_rate;
    double m_mutation_rate;
    double m_crossover_rate;
    double m_reproduction_rate;
    rng_type &m_rng;
    fitness_function_type m_fitness_function;
    mutation_type m_mutation_function;
    crossover_type m_crossover_function;
    reproduction_type m_reproduction_function;
};




} // namespace gp


#endif // GP_GA_GA2_HPP_DEFINED
