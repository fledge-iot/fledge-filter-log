#ifndef _LOG_FILTER_H
#define _LOG_FILTER_H
/*
 * Fledge "Log" filter plugin.
 *
 * Copyright (c) 2020 Dianomic Systems
 *
 * Released under the Apache 2.0 Licence
 *
 * Author: Mark Riddoch           
 */     
#include <filter.h>               
#include <reading_set.h>
#include <config_category.h>
#include <string>                 
#include <logger.h>
#include <mutex>
#include <regex>
#include <math.h>


/**
 * Convert the incoming data to use a logarithmic scale
 */
class LogFilter : public FledgeFilter {
	public:
		LogFilter(const std::string& filterName,
                        ConfigCategory& filterConfig,
                        OUTPUT_HANDLE *outHandle,
                        OUTPUT_STREAM output);
		~LogFilter();
		void	ingest(READINGSET *readingSet);
		void	reconfigure(const std::string& newConfig);
	private:
		void				handleConfig(ConfigCategory& config);
		std::string			m_match;
		std::regex			*m_regex;
		std::mutex			m_configMutex;
};


#endif
