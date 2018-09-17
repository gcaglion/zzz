#ifndef info
	#define info(msgMask_, ...) dbg->out(DBG_MSG_INFO, __func__, depth, msgMask_, __VA_ARGS__)
#endif
#ifndef err
	#define err(msgMask_, ...) dbg->out(DBG_MSG_ERR, __func__, depth, msgMask_, __VA_ARGS__)
#endif
#ifndef fail
	#define fail(msgMask_, ...) { dbg->out(DBG_MSG_ERR, __func__, depth, msgMask_, __VA_ARGS__); throw std::exception(dbg->msg);}
#endif


