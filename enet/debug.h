/**
 * @author Edouard DUPIN
 * 
 * @copyright 2014, Edouard DUPIN, all right reserved
 * 
 * @license APACHE v2.0 (see license file)
 */
#pragma once

#include <elog/log.h>

namespace enet {
	int32_t getLogId();
};

#define ENET_BASE(info,data) ELOG_BASE(enet::getLogId(),info,data)

#define ENET_CRITICAL(data)      ENET_BASE(1, data)
#define ENET_ERROR(data)         ENET_BASE(2, data)
#define ENET_WARNING(data)       ENET_BASE(3, data)
#ifdef DEBUG
	#define ENET_INFO(data)          ENET_BASE(4, data)
	#define ENET_DEBUG(data)         ENET_BASE(5, data)
	#define ENET_VERBOSE(data)       ENET_BASE(6, data)
	#define ENET_TODO(data)          ENET_BASE(4, "TODO : " << data)
#else
	#define ENET_INFO(data)          do { } while(false)
	#define ENET_DEBUG(data)         do { } while(false)
	#define ENET_VERBOSE(data)       do { } while(false)
	#define ENET_TODO(data)          do { } while(false)
#endif

#define ENET_ASSERT(cond,data) \
	do { \
		if (!(cond)) { \
			ENET_CRITICAL(data); \
			assert(!#cond); \
		} \
	} while (0)


