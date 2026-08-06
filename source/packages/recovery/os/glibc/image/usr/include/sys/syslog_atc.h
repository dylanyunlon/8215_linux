/*
 *      Macro definiton related to syslog
 *	@(#)syslog_atc.h
 */

#ifndef _SYS_SYSLOG_ATC_H
#define _SYS_SYSLOG_ATC_H 1

#define  DA_DEBUG	1
#define SYSLOG_WITH_TAG 1
#ifdef DA_DEBUG

/*
 * Opens a connection to the system logger for a progarm.
 */
#define DOPENLOG(tag)			openlog(tag, LOG_PID | LOG_CONS | LOG_PERROR, LOG_USER)
#define aopenlog(tag)			openlog(tag, LOG_PID | LOG_CONS | LOG_PERROR, LOG_USER)

/*
 * Closes the descriptor.
 */
#define DCLOSELOG()			closelog()
#define acloselog()			closelog()

/*
 * set the mask of all priorities in the above list up to and including lvl.
 */
#define DMASKLOG(lvl)			setlogmask(LOG_UPTO(lvl))
#define amasklog(lvl)			setlogmask(LOG_UPTO(lvl))

#ifdef SYSLOG_WITH_TAG
/*
 * Not only generate a log message using FMT string and option arguments,
 * But also with Specified Tag Name, File Name, function name and line number.
 */
#define DSYSLOG_EN(tag, lvl, format, ...)	syslog(lvl, "[%s], File: %s, Function: %s, Line: %d " format, \
						tag, (char *)__FILE__, (char *)__func__, (int)__LINE__, ##__VA_ARGS__)

#define DSYSLOG(tag, lvl, format, ...)		syslog(lvl, "[%s] " format, tag, ##__VA_ARGS__)

#define asyslog(tag, lvl, format, ...)		syslog(lvl, "[%s] " format, tag, ##__VA_ARGS__)
#define asyslogd(tag, format, ...)		syslog(LOG_DEBUG, "[%s] " format, tag, ##__VA_ARGS__)
#define asyslogi(tag, format, ...)		syslog(LOG_INFO, "[%s] " format, tag, ##__VA_ARGS__)
#define asyslogn(tag, format, ...)		syslog(LOG_NOTICE, "[%s] " format, tag, ##__VA_ARGS__)
#define asyslogw(tag, format, ...)		syslog(LOG_WARNING, "[%s], File: %s, Function: %s, Line: %d " format, \
							tag, (char *)__FILE__, (char *)__func__, (int)__LINE__, ##__VA_ARGS__)
#define asysloge(tag, format, ...)		syslog(LOG_ERR, "[%s], File: %s, Function: %s, Line: %d " format, \
							tag, (char *)__FILE__, (char *)__func__, (int)__LINE__, ##__VA_ARGS__)
#define asyslogc(tag, format, ...)		syslog(LOG_CRIT, "[%s], File: %s, Function: %s, Line: %d " format, \
							tag, (char *)__FILE__, (char *)__func__, (int)__LINE__, ##__VA_ARGS__)
#define asysloga(tag, format, ...)		syslog(LOG_ALERT, "[%s], File: %s, Function: %s, Line: %d " format, \
							tag, (char *)__FILE__, (char *)__func__, (int)__LINE__, ##__VA_ARGS__)
#define asyslogm(tag, format, ...)		syslog(LOG_EMERG, "[%s], File: %s, Function: %s, Line: %d " format, \
							tag, (char *)__FILE__, (char *)__func__, (int)__LINE__, ##__VA_ARGS__)
#else  /* SYSLOG_WITH_TAG */
/*
 * Not only generate a log message using FMT string and option arguments,
 * But also with File Name, function name and line number.
 */
#define DSYSLOG(lvl, format, ...)	syslog(lvl, "File: %s, Function: %s, Line: %d "format, \
					(char *)__FILE__, (char *)__func__, (int)__LINE__, ##__VA_ARGS__)
#define asyslog(lvl, format, ...)	syslog(lvl, format, ##__VA_ARGS__)
#define asyslogd(format, ...)		syslog(LOG_DEBUG, format, ##__VA_ARGS__)
#define asyslogi(format, ...)		syslog(LOG_INFO, format, ##__VA_ARGS__)
#define asyslogn(format, ...)		syslog(LOG_NOTICE, format, ##__VA_ARGS__)
#define asyslogw(format, ...)		syslog(LOG_WARNING, format, ##__VA_ARGS__)
#define asysloge(format, ...)		syslog(LOG_ERR, format, ##__VA_ARGS__)
#define asyslogc(format, ...)		syslog(LOG_CRIT, format, ##__VA_ARGS__)
#define asysloga(format, ...)		syslog(LOG_ALERT, format, ##__VA_ARGS__)
#define asyslogm(format, ...)		syslog(LOG_EMERG, format, ##__VA_ARGS__)

#endif /* SYSLOG_WITH_TAG */


#else   /*  DA_DEBUG */

#define DOPENLOG(tag, opt, com)		openlog(tag, opt, com)
#define aopenlog(tag)			openlog(tag, LOG_PID | LOG_CONS, LOG_USER)
#define DSYSLOG(lvl, format, ...)	syslog(lvl, format, ##__VA_ARGS__)
#define DSYSLOG_EN(tag, lvl, format, ...)	syslog(lvl, format, ##__VA_ARGS__)
#define DCLOSELOG()			closelog()
#define acloselog()			closelog()
#define DMASKLOG(lvl)			setlogmask(lvl)
#define amasklog(lvl)			setlogmask(lvl)

#ifdef SYSLOG_WITH_TAG
#define asyslog(tag, lvl, format, ...)	syslog(lvl, format, ##__VA_ARGS__)
#define asyslogd(tag, format, ...)	syslog(LOG_DEBUG, format, ##__VA_ARGS__)
#define asyslogi(tag, format, ...)	syslog(LOG_INFO, format, ##__VA_ARGS__)
#define asyslogn(tag, format, ...)	syslog(LOG_NOTICE, format, ##__VA_ARGS__)
#define asyslogw(tag, format, ...)	syslog(LOG_WARNING, format, ##__VA_ARGS__)
#define asysloge(tag, format, ...)	syslog(LOG_ERR, format, ##__VA_ARGS__)
#define asyslogc(tag, format, ...)	syslog(LOG_CRIT, format, ##__VA_ARGS__)
#define asysloga(tag, format, ...)	syslog(LOG_ALERT, format, ##__VA_ARGS__)
#define asyslogm(tag, format, ...)	syslog(LOG_EMERG, format, ##__VA_ARGS__)

#else  /* SYSLOG_WITH_TAG */
#define asyslog(lvl, format, ...)	syslog(lvl, format, ##__VA_ARGS__)
#define asyslogd(format, ...)	syslog(LOG_DEBUG, format, ##__VA_ARGS__)
#define asyslogi(format, ...)	syslog(LOG_INFO, format, ##__VA_ARGS__)
#define asyslogn(format, ...)	syslog(LOG_NOTICE, format, ##__VA_ARGS__)
#define asyslogw(format, ...)	syslog(LOG_WARNING, format, ##__VA_ARGS__)
#define asysloge(format, ...)	syslog(LOG_ERR, format, ##__VA_ARGS__)
#define asyslogc(format, ...)	syslog(LOG_CRIT, format, ##__VA_ARGS__)
#define asysloga(format, ...)	syslog(LOG_ALERT, format, ##__VA_ARGS__)
#define asyslogm(format, ...)	syslog(LOG_EMERG, format, ##__VA_ARGS__)
#endif /* SYSLOG_WITH_TAG */


#endif /* DA_DEBUG */


#endif /* sys/syslog_atc.h */
