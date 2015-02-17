/*
 * Asterisk -- An open source telephony toolkit.
 *
 * Copyright (C) 2009, Digium, Inc.
 *
 * Steve Murphy <murf@digium.com>
 * much borrowed from cdr code (cdr_custom.c), author Mark Spencer
 *
 * See http://www.asterisk.org for more information about
 * the Asterisk project. Please do not directly contact
 * any of the maintainers of this project for assistance;
 * the project provides a web site, mailing lists and IRC
 * channels for your use.
 *
 * This program is free software, distributed under the terms of
 * the GNU General Public License Version 2. See the LICENSE file
 * at the top of the source tree.
 */
/*** MODULEINFO
	<depend>CURL</depend>
 ***/


#include "asterisk.h"

ASTERISK_FILE_VERSION(__FILE__, "$Revision: 419592 $")

#include "asterisk/module.h"
#include "asterisk/logger.h"
#include "asterisk/cel.h"

#include <curl/curl.h>


#define AST_MODULE "cel_http"
#define CUSTOM_BACKEND_NAME "CEL Custom HTTP Logging"

AST_THREADSTORAGE(custom_buf);

static const char DATE_FORMAT[] = "%Y-%m-%d %T";
static const char name[] = "cel-http";

static int unload_module(void)
{
	ast_log(LOG_NOTICE, "cya");
	return 0;
}

static void custom_log(struct ast_event *event)
{
	CURL *curl;
	CURLcode res;
	struct ast_tm timeresult;
	struct curl_slist *headers = NULL;
	char start_time[80] = "";

	struct ast_cel_event_record record = {
		.version = AST_CEL_EVENT_RECORD_VERSION,
	};

	if (ast_cel_fill_record(event, &record)) {
		return;
	}

	ast_localtime(&record.event_time, &timeresult, NULL);
	ast_strftime(start_time, sizeof(start_time), DATE_FORMAT, &timeresult);

	ast_log(LOG_NOTICE, "EventName: %s\r\n", record.user_defined_name);
	ast_log(LOG_NOTICE, "CalledIDName : %s\r\n", record.caller_id_name);

	ast_log(LOG_NOTICE, "Hello here");
	curl = curl_easy_init();
	if (curl) {
		headers = curl_slist_append(headers, "Accept: application/json");
		headers = curl_slist_append(headers, "Content-Type: application/json");
		ast_log(LOG_NOTICE, "Headers");

		curl_easy_setopt(curl, CURLOPT_URL, "http://192.168.0.5:9200/blah/user/");
		curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
		curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "PUT");
		curl_easy_setopt(curl, CURLOPT_POSTFIELDS, "{ \"name\" : \"Anonymous\"}");
		ast_log(LOG_NOTICE, "OK Sending");

		res = curl_easy_perform(curl);
		curl_slist_free_all(headers);
		curl_easy_cleanup(curl);
	}
}

static int load_module(void)
{

	if (ast_cel_backend_register(CUSTOM_BACKEND_NAME, custom_log)) {
		return AST_MODULE_LOAD_FAILURE;
	}

	ast_log(LOG_NOTICE, "Loading HTTP CEL");
	return AST_MODULE_LOAD_SUCCESS;
}


AST_MODULE_INFO(ASTERISK_GPL_KEY, AST_MODFLAG_DEFAULT, "Hello World Stuff",
	.load = load_module,
	.unload = unload_module
	);
