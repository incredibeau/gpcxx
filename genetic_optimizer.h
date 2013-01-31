/*
 * genetic_optimizer.h
 * Date: 2013-01-27
 * Author: Karsten Ahnert (karsten.ahnert@gmx.de)
 */

#ifndef GENETIC_OPTIMIZER_H_INCLUDED
#define GENETIC_OPTIMIZER_H_INCLUDED

#include "tree.h"
#include "tree_io.h"
#include "evaluation.h"
#include "generate_random_tree.h"
#include "mutation.h"
#include "cross_over.h"
#include "helpers.h"
#include "fitness.h"

#include <random>
#include <iostream>
using namespace std;
/*
Algorithm

initialize random trees
for i:
  evaluate fitness
  reproduce
  mutate
  crossover
*/

#include <random>

class genetic_optimizer
{
public:

    typedef std::vector< double > vector_t;
    typedef std::vector< size_t > index_vector_t;
    typedef tree< char > pheno_t;
    typedef std::vector< pheno_t > population_t;
    typedef std::vector< double > fitness_vector_t;

    genetic_optimizer( size_t population_size , size_t min_height , size_t max_height ,
                       double reproduction_rate , double mutation_rate , double crossover_rate )
        : m_pop() , m_fitness() ,
          m_min_height( min_height ) , m_max_height( max_height ) ,
          m_reproduction_rate( reproduction_rate ) , m_mutation_rate( mutation_rate ) , m_crossover_rate( crossover_rate )
    {
        initialize( population_size );
    }


    void calc_fitness( const vector_t &y , const vector_t &x1 , const vector_t &x2 , const vector_t &x3 )
    {
        for( size_t i=0 ; i<m_pop.size() ; ++i )
            m_fitness_vector[i] = fitness_t::fitness( m_pop[i] , y , x1 , x2 , x3 );
    }

    void iterate( const vector_t &y , const vector_t &x1 , const vector_t &x2 , const vector_t &x3 )
    {
        reproduce( m_pop , m_fitness_vector );
        mutate( m_pop );
        cross_over( m_pop );
        calc_fitness( y , x1 , x2 , x3 );
    }

    void reproduce( population_t &p , fitness_vector_t &fitness )
    {
        index_vector_t idx;
        auto iter = sort_indexes( fitness , idx );
        size_t n_repro = size_t( double( m_pop.size() ) * m_reproduction_rate );
        auto iter2 = std::min( idx.begin() + n_repro , iter );
        population_t tmp_pop;
        for_each( iter2 , iter2 + 20 , [&]( size_t i ) { tmp_pop.push_back( m_pop[ idx[i] ] ); } );
        std::uniform_int_distribution< size_t > dist( 0 , iter2 - idx.begin() - 1 );
        for_each( iter2 , idx.end() , [&]( size_t i ) { m_pop[i] = m_pop[ idx[ dist(m_fitness.m_rng) ] ]; } );
        std::uniform_int_distribution< size_t > dist2( 0 , m_pop.size() - 1);
        for_each( tmp_pop.begin() , tmp_pop.end() , [&]( const pheno_t &t ) { m_pop[ dist2(m_fitness.m_rng) ] = t; } );
    }

    void mutate( population_t &p )
    {
        size_t n_mut = size_t( double( m_pop.size() ) * m_mutation_rate );
        index_vector_t idx;
        create_random_indexes( idx , m_pop.size() , n_mut );
        random_symbol_generator< char , std::mt19937 > terminal_gen( m_fitness.m_eval.terminal_symbols , m_fitness.m_rng , 0 );
        random_symbol_generator< char , std::mt19937 > unary_gen( m_fitness.m_eval.unary_symbols , m_fitness.m_rng , 1 );
        random_symbol_generator< char , std::mt19937 > binary_gen( m_fitness.m_eval.binary_symbols , m_fitness.m_rng , 2 );
        for( size_t i=0 ; i<idx.size() ; ++i )
            mutation( m_pop[ idx[i] ] , m_fitness.m_rng , terminal_gen , unary_gen , binary_gen );
    }

    void cross_over( population_t &p )
    {
        size_t n_cross = size_t( double( m_pop.size() ) * m_crossover_rate );
        index_vector_t idx;
        create_random_indexes( idx , m_pop.size() , n_cross );
        for( size_t i=0 ; i<idx.size() ; i+=2 )
            ::cross_over( m_pop[ idx[i] ] , m_pop[ idx[i+1] ] , m_fitness.m_rng , m_max_height );
    }

    void report_population( std::ostream &out )
    {
        index_vector_t idx;
        auto iter = sort_indexes( m_fitness_vector , idx );
        for( size_t i=0 ; i<10 ; ++i )
        {
            out << i << " " << m_fitness_vector[ idx[i] ] << " : ";
            print_formula( m_pop[ idx[i] ] , out );
            out << endl;
        }
        out << endl;
    }

    const fitness_vector_t fitness_vector( void ) const { return m_fitness_vector; }
    const population_t& population( void ) const { return m_pop; }


private:

    void initialize( size_t pop_size )
    {
        random_symbol_generator< char , std::mt19937 > terminal_gen( m_fitness.m_eval.terminal_symbols , m_fitness.m_rng , 0 );
        random_symbol_generator< char , std::mt19937 > unary_gen( m_fitness.m_eval.unary_symbols , m_fitness.m_rng , 1 );
        random_symbol_generator< char , std::mt19937 > binary_gen( m_fitness.m_eval.binary_symbols , m_fitness.m_rng , 2 );

        m_pop.resize( pop_size );
        m_fitness_vector.resize( pop_size );
        for( size_t i=0 ; i<pop_size ; ++i )
            generate_random_tree( m_pop[i] , unary_gen , binary_gen , terminal_gen , m_fitness.m_rng , m_min_height , m_max_height );
    }

    population_t m_pop;
    fitness_t m_fitness;
    fitness_vector_t m_fitness_vector;
    size_t m_min_height , m_max_height;
    double m_reproduction_rate , m_mutation_rate , m_crossover_rate;
};

#endif // GENETIC_OPTIMIZER_H_INCLUDED
