#ifndef STATIC_VARS_H
#define STATIC_VARS_H

#include <boost/utility.hpp>
#include <vector>
#include "eraser/common.h"
#include "eraser/logger.h"

namespace eraser
{

struct static_vars : boost::noncopyable
{
	typedef jvmti_traits::field_id_t  fields_key_t;
	typedef std::vector<shared_var_t*> map_t;

	static static_vars* instance()
	{
		static static_vars instance_;
		return &instance_;
	}

	shared_var_t* get_shared_var( fields_key_t field )
	{
		for( size_t j=0; j<vars_.size(); ++j )
			if( vars_[j]->field_id_	== field )
				return vars_[j];
		return 0;
	}
	void put_shared_var( shared_var_t* shared_var )
	{
		vars_.push_back( shared_var );
		logger::instance()->level(5) << "STATIC VAR FSM INIT" << std::endl;
	}

	map_t vars_;
	private: static_vars(){}
};


}

#endif
