/*************************************************************************
 * PLEASE SEE THE FILE "license.txt" (INCLUDED WITH THIS SOFTWARE PACKAGE)
 * FOR LICENSE AND COPYRIGHT INFORMATION. 
 *************************************************************************/

/*************************************************************************
 *
 *  file:  soar_module.h
 *
 * =======================================================================
 */

#ifndef SOAR_MODULE_H
#define SOAR_MODULE_H

#include <map>
#include <string>

#include "misc.h"

// separates this functionality
// just for Soar modules
namespace soar_module
{
	///////////////////////////////////////////////////////////////////////////
	// Predicates
	///////////////////////////////////////////////////////////////////////////
	
	// a functor for validating parameter values
	template <typename T>
	class predicate
	{
		public:
			virtual ~predicate();			
			virtual bool operator() ( T val );
	};

	// a false predicate
	template <typename T>
	class f_predicate: public predicate<T>
	{
		public:			
			virtual bool operator() ( T val );
	};
	
	// predefined predicate for validating
	// a value between two values known at
	// predicate initialization
	template <typename T>
	class btw_predicate: public predicate<T>
	{
		private:
			T my_min;
			T my_max;
			bool inclusive;
		
		public:
			btw_predicate( T new_min, T new_max, bool new_inclusive );		
			bool operator() ( T val );
	};
	
	// predefined predicate for validating
	// a value greater than a value known at
	// predicate initialization
	template <typename T>
	class gt_predicate: public predicate<T>
	{
		private:
			T my_min;			
			bool inclusive;
		
		public:
			gt_predicate( T new_min, bool new_inclusive );		
			bool operator() ( T val );
	};
	
	// predefined predicate for validating
	// a value less than a value known at
	// predicate initialization
	template <typename T>
	class lt_predicate: public predicate<T>
	{
		private:
			T my_max;			
			bool inclusive;
		
		public:
			lt_predicate( T new_max, bool new_inclusive );		
			bool operator() ( T val );
	};

	// superclass for predicates needing
	// agent state
	template <typename T>
	class agent_predicate: public predicate<T>
	{
		protected:
			agent *my_agent;

		public:
			agent_predicate( agent *new_agent );
	};

	///////////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////
	
	
	///////////////////////////////////////////////////////////////////////////
	// Parameters
	///////////////////////////////////////////////////////////////////////////
	
	// all parameters have a name and
	// can be manipulated generically
	// via strings
	class param
	{
		private:
			const char *name;
			
		public:		
			param( const char *new_name );
			virtual ~param();

			// 

			const char *get_name();
			
			//
			
			virtual char *get_string() = 0;
			virtual bool set_string( const char *new_string ) = 0;
			virtual bool validate_string( const char *new_string ) = 0;			
	};
	
	// a primitive parameter can take any primitive
	// data type as value and is validated via
	// any unary predicate
	template <typename T>
	class primitive_param: public param
	{
		protected:
			T value;
			predicate<T> *val_pred;
			predicate<T> *prot_pred;
		
		public:
			primitive_param( const char *new_name, T new_value, predicate<T> *new_val_pred, predicate<T> *new_prot_pred );			
			virtual ~primitive_param();
			
			//
			
			virtual char *get_string();			
			virtual bool set_string( const char *new_string );			
			virtual bool validate_string( const char *new_string );
			
			//
			
			virtual T get_value();
			virtual void set_value( T new_value );
	};
	
	// these are easy definitions for int and double parameters
	typedef primitive_param<long> integer_param;
	typedef primitive_param<double> decimal_param;
	

	// a string param deals with character strings
	class string_param: public param
	{
		protected:
			std::string *value;
			predicate<const char *> *val_pred;
			predicate<const char *> *prot_pred;
		
		public:
			string_param( const char *new_name, const char *new_value, predicate<const char *> *new_val_pred, predicate<const char *> *new_prot_pred );			
			virtual ~string_param();
			
			//
			
			virtual char *get_string();
			virtual bool set_string( const char *new_string );
			virtual bool validate_string( const char *new_value );
			
			//
			
			virtual const char *get_value();
			virtual void set_value( const char *new_value );
	};
	
	// a constant parameter deals in discrete values
	// for efficiency, internally we use enums, elsewhere
	// strings for user-readability
	template <typename T>
	class constant_param: public param
	{
		protected:		
			T value;
			std::map<T, const char *> *value_to_string;
			std::map<std::string, T> *string_to_value;
			predicate<T> *prot_pred;			
			
		public:						
			constant_param( const char *new_name, T new_value, predicate<T> *new_prot_pred );			
			virtual ~constant_param();		
			
			//
			
			virtual char *get_string();			
			virtual bool set_string( const char *new_string );			
			virtual bool validate_string( const char *new_string );

			//
			
			virtual T get_value();
			virtual void set_value( T new_value );
			
			//
			
			virtual void add_mapping( T val, const char *str );
	};

	// this is an easy implementation of a boolean parameter
	enum boolean { off, on };
	class boolean_param: public constant_param<boolean>
	{
		public:
			boolean_param( const char *new_name, boolean new_value, predicate<boolean> *new_prot_pred );
	};
	
	///////////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////
	

	///////////////////////////////////////////////////////////////////////////
	// Parameter Containers
	///////////////////////////////////////////////////////////////////////////

	// shortcut definition
	typedef std::map<std::string, param *> param_map;
	
	// this class provides for efficient 
	// string->parameter access
	class param_container
	{
		private:
			param_map *params;
			
		protected:			
			agent *my_agent;
			
			void add_param( param *new_param );
			
		public:
			param_container( agent *new_agent );			
			virtual ~param_container();

			param *get_param( const char *name );
	};

	///////////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////

	///////////////////////////////////////////////////////////////////////////
	// Statistics
	///////////////////////////////////////////////////////////////////////////
	
	// all statistics have a name and
	// can be retrieved generically
	// via strings
	class stat
	{
		private:
			const char *name;
			
		public:		
			stat( const char *new_name );
			virtual ~stat();
			
			// 
					
			const char *get_name();
			
			//
			
			virtual char *get_string() = 0;
			virtual void reset() = 0;
	};

	// a primitive statistic can take any primitive
	// data type as value
	template <typename T>
	class primitive_stat: public stat
	{
		private:
			T value;
			T reset_val;
			predicate<T> *prot_pred;
		
		public:
			primitive_stat( const char *new_name, T new_value, predicate<T> *new_prot_pred );			
			virtual ~primitive_stat();
			
			//
			
			virtual char *get_string();
			void reset();
			
			//
			
			virtual T get_value();
			virtual void set_value( T new_value );			
	};
	
	// these are easy definitions for int and double parameters
	typedef primitive_stat<long> integer_stat;
	typedef primitive_stat<double> decimal_stat;

	///////////////////////////////////////////////////////////////////////////
	// Statistic Containers
	///////////////////////////////////////////////////////////////////////////

	// shortcut definition
	typedef std::map<std::string, stat *> stat_map;
	
	// this class provides for efficient 
	// string->stat access
	class stat_container
	{
		private:
			stat_map *stats;
			
		protected:			
			agent *my_agent;
			
			void add_stat( stat *new_stat );
			
		public:
			stat_container( agent *new_agent );			
			virtual ~stat_container();
			
			stat *get_stat( const char *name );
			void reset_stats();
	};
}

#endif