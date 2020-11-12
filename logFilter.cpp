/*
 * Fledge "Log" filter plugin.
 *
 * Copyright (c) 2020 Dianomic Systems
 *
 * Released under the Apache 2.0 Licence
 *
 * Author: Mark Riddoch           
 */     
#include <logFilter.h>               

using namespace std;

/**
 * Constructor for the LogFilter.
 *
 * We call the constructor of the base class and handle the initial 
 * configuration of the filter.
 *
 * @param	filterName	The name of the filter
 * @param	filterConfig	The configuration category for this filter
 * @param	outHandle	The handle ofthe next filter in the chain
 * @param	output		A function pointer to call to output data to the next filter
 */
LogFilter::LogFilter(const std::string& filterName,
                        ConfigCategory& filterConfig,
                        OUTPUT_HANDLE *outHandle,
                        OUTPUT_STREAM output) : m_regex(NULL),
				FledgeFilter(filterName, filterConfig, outHandle, output)
{
	handleConfig(filterConfig);
}

/**
 * Destructor for this filter class
 */
LogFilter::~LogFilter()
{
	if (m_regex)
		delete m_regex;
}

/**
 * The actual filtering code
 *
 * @param readingSet	The reading data to filter
 */
void
LogFilter::ingest(READINGSET *readingSet)
{
	lock_guard<mutex> guard(m_configMutex);

	if (isEnabled())	// Filter enable, process the readings
	{
		const vector<Reading *>& readings = ((ReadingSet *)readingSet)->getAllReadings();
		for (vector<Reading *>::const_iterator elem = readings.begin();
				elem != readings.end(); ++elem)
		{
			// If we set a matching regex then compare to the name of this asset
			if (!m_match.empty())
			{
				string asset = (*elem)->getAssetName();
				if (!regex_match(asset, *m_regex))
				{
					continue;
				}
			}

			// We are modifying this asset so put an entry in the asset tracker
			AssetTracker::getAssetTracker()->addAssetTrackingTuple(getName(), (*elem)->getAssetName(), string("Filter"));

			// Get a reading DataPoints
			const vector<Datapoint *>& dataPoints = (*elem)->getReadingData();

			// Iterate over the datapoints
			for (vector<Datapoint *>::const_iterator it = dataPoints.begin(); it != dataPoints.end(); ++it)
			{
				// Get the reference to a DataPointValue
				DatapointValue& value = (*it)->getData();

				/*
				 * Deal with the T_INTEGER and T_FLOAT types.
				 * Try to preserve the type if possible but
				 * if a floating point log function is applied
				 * then T_INTEGER values will turn into T_FLOAT.
				 * If the value is zero we do not apply the log function
				 */
				if (value.getType() == DatapointValue::T_INTEGER)
				{
					long ival = value.toInt();
					if (ival != 0)
					{
						double newValue = log((double)ival);
						value.setValue(newValue);
					}
				}
				else if (value.getType() == DatapointValue::T_FLOAT)
				{
					double dval = value.toDouble();
					if (dval != 0.0)
					{
						value.setValue(log(dval));
					}
				}
				else
				{
					// do nothing for other types
				}
			}
		}
	}

	// Pass on all readings in this case
	(*m_func)(m_data, readingSet);
}

/**
 * Reconfiguration entry point to the filter.
 *
 * This method runs holding the configMutex to prevent
 * ingest using the regex class that may be destroyed by this
 * call.
 *
 * Pass the configuration to the base FilterPlugin class and
 * then call the private method to handle the filter specific 
 * configuration.
 *
 * @param newConfig	The JSON of the new configuration
 */
void
LogFilter::reconfigure(const std::string& newConfig)
{
	lock_guard<mutex> guard(m_configMutex);
	setConfig(newConfig);		// Pass the configuration to the base class
	handleConfig(m_config);
}

/**
 * Handle the filter specific configuration. In this case
 * it is just the single item "match" that is a regex
 * expression
 *
 * @param config	The configuration category
 */
void
LogFilter::handleConfig(ConfigCategory& config)
{
	if (config.itemExists("match"))
	{
		m_match = config.getValue("match");
		if (m_regex)
			delete m_regex;
		m_regex = new regex(m_match);
	}
}
