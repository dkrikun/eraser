
#include "eraser/lock.h"
#include "eraser/agent.h"
#include "eraser/logger.h"
#include "sun/agent_util.h"

namespace eraser
{

bool lock::operator==( const lock& rhs ) const
{
	scoped_lock( agent::instance()->jvmti(), agent::instance()->monitor_ );
	bool res = ( agent::instance()->jni()->IsSameObject( lock_id_, rhs.lock_id_ ) == JNI_TRUE );
	logger::instance()->level(1) << INFO << "lock compare res: " << std::boolalpha << res << std::endl;
	return res;
}


inline std::size_t hash_value( const lock& l )
{
	jvmtiError err;
	jint hash_code;
	err = agent::instance()->jvmti()->GetObjectHashCode( l.lock_id_, &hash_code );
	check_jvmti_error(agent::instance()->jvmti(), err, "get object hash code");
	std::size_t res = static_cast<std::size_t>(hash_code);
	logger::instance()->level(1) << INFO << "lock hash=" << res << std::endl;
	return res;
}

}




