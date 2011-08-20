
#include "eraser/lock.h"
#include "eraser/agent.h"
#include "eraser/logger.h"
#include "sun/agent_util.h"

namespace eraser
{

bool lock::operator==( const lock& rhs ) const
{
	bool res = ( agent::instance()->jni()->IsSameObject( lock_id_, rhs.lock_id_ ) == JNI_TRUE );
	LOG_DEBUG( "lock compare res: " << std::boolalpha << res, dummy );
	return res;
}


inline std::size_t hash_value( const lock& l )
{
	jvmtiError err;
	jint hash_code;
	err = agent::instance()->jvmti()->GetObjectHashCode( l.lock_id_, &hash_code );
	check_jvmti_error(agent::instance()->jvmti(), err, "get object hash code");
	std::size_t res = static_cast<std::size_t>(hash_code);
	LOG_DEBUG( "lock hash=" << res, dummy );
	return res;
}

}




