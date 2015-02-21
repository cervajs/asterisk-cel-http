/*
 * Asterisk -- An open source telephony toolkit.
 *
 * Copyright (C) 2015, House of the Hat, where the baby cries 
 * and the mother dont see.
 *
 * Amim Knabben - amim.knabben@gmail.com
 */

/*** MODULEINFO
	<depend>CURL</depend>
 ***/


#include "asterisk.h"

ASTERISK_FILE_VERSION(__FILE__, "$Revision: 1 $")

#include "asterisk/module.h"
#include "asterisk/logger.h"
#include "asterisk/cel.h"
#include "asterisk/json.h"

#include <curl/curl.h>


#define AST_MODULE "cel_http"
#define CUSTOM_BACKEND_NAME "CEL Custom HTTP Logging"


AST_THREADSTORAGE(custom_buf);

static const char DATE_FORMAT[] = "%Y-%m-%d %T";
static const char name[] = "cel-http";

static int unload_module(void)
{
	ast_log(LOG_NOTICE, "Unloading CEL HTTP Module");
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

	curl = curl_easy_init();
	if (curl) {
		headers = curl_slist_append(headers, "Content-Type: application/json");

		curl_easy_setopt(curl, CURLOPT_URL, "http://192.168.0.5:9200/asterisk/cel/");
		curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

		char *post_fields = malloc(sizeof(char) * 6048);
		sprintf(post_fields, "{\"EventName\": \"%s\", \"AccountCode\": \"%s\",	\
							\"CallerIDnum\": \"%s\", \"CallerIDname\": \"%s\", 	\
							\"CallerIDani\": \"%s\", \"CallerIDrdnis\": \"%s\",	\
							\"CAllerIDdnid\": \"%s\", \"Exten\": \"%s\",		\
							\"Context\": \"%s\", \"Channel\": \"%s\", 	 		\
							\"Application\": \"%s\", \"AppData\": \"%s\",		\
							\"EventTime\": \"%s\", \"AMAFlags\": \"%s\", 	 	\
							\"UniqueID\": \"%s\", \"LinkedID\": \"%s\", 	 	\
							\"Userfield\": \"%s\", \"Peer\": \"%s\", 	 		\
							\"Peeraccount\": \"%s\", \"Extra\": \"%s\" }",
							record.event_name, record.account_code, record.caller_id_num,
							record.caller_id_name, record.caller_id_ani, record.caller_id_rdnis,
							record.caller_id_dnid, record.extension, record.context,
							record.channel_name, record.application_name, record.application_data,
							start_time, ast_channel_amaflags2string(record.amaflag),
							record.unique_id, record.linked_id, record.user_field, record.peer,
							record.peer_account, record.extra);

		curl_easy_setopt(curl, CURLOPT_POSTFIELDS, post_fields);

		curl_easy_perform(curl);
		curl_slist_free_all(headers);
		curl_easy_cleanup(curl);
		free(post_fields);
	}
}

static int load_module(void)
{

	if (ast_cel_backend_register(CUSTOM_BACKEND_NAME, custom_log)) {
		return AST_MODULE_LOAD_FAILURE;
	}

	ast_log(LOG_NOTICE, "Loading HTTP CEL Module");
	return AST_MODULE_LOAD_SUCCESS;
}


AST_MODULE_INFO(ASTERISK_GPL_KEY, AST_MODFLAG_DEFAULT, "Hello World Stuff",
	.load = load_module,
	.unload = unload_module
	);
