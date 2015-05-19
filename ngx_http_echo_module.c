#include <ngx_config.h>
#include <ngx_core.h>
#include <ngx_http.h>

/* Module config */
typedef struct {
    ngx_str_t  ed;
} ngx_http_echo_loc_conf_t;

static char *ngx_http_echo(ngx_conf_t *cf, ngx_command_t *cmd, void *conf);
static void *ngx_http_echo_create_loc_conf(ngx_conf_t *cf);
static char *ngx_http_echo_merge_loc_conf(ngx_conf_t *cf, void *parent, void *child);
/* Directives */
static ngx_command_t  ngx_http_echo_commands[] = {
	{
		ngx_string("echo"),
		NGX_HTTP_LOC_CONF|NGX_CONF_TAKE1,
		ngx_http_echo,
		NGX_HTTP_LOC_CONF_OFFSET,
		offsetof(ngx_http_echo_loc_conf_t, ed),
		NULL
	},
	ngx_null_command
};

/* Http context of the module */
static ngx_http_module_t  ngx_http_echo_module_ctx = {
    NULL,                                  /* preconfiguration */               //在解析配置文件中http{}配置块前调用 ngx_int_t (*preconfiguration)(ngx_conf_t *cf);
    NULL,                                  /* postconfiguration */              //在解析配置文件中http{}配置块后调用 ngx_int_t (*postconfiguration)(ngx_conf_t *cf);
    NULL,                                  /* create main configuration */      //创建http模块的server config    void *(*create_main_conf)(ngx_conf_t *cf);
    NULL,                                  /* init main configuration */        //初始化http模块的main config     char *(*init_main_conf)(ngx_conf_t *cf, void *conf);
    NULL,                                  /* create server configuration */	//创建http模块的server config    void *(*create_srv_conf)(ngx_conf_t *cf);
    NULL,                                  /* merge server configuration */     //合并http模块的server config，用于实现server config到main config的指令的继承、覆盖 char *(*merge_srv_conf)(ngx_conf_t *cf, void *prev, void *conf);
    ngx_http_echo_create_loc_conf,         /* create location configration */   //创建http模块的location config  void *(*create_loc_conf)(ngx_conf_t *cf);
    ngx_http_echo_merge_loc_conf           /* merge location configration */    //合并http模块的location config，用于实现location config到server config的指令的继承、覆盖 char *(*merge_loc_conf)(ngx_conf_t *cf, void *prev, void *conf);
};
/* Module */
ngx_module_t  ngx_http_echo_module = {
    NGX_MODULE_V1,
    &ngx_http_echo_module_ctx,             /* module context */
    ngx_http_echo_commands,                /* module directives */
    NGX_HTTP_MODULE,                       /* module type */
    NULL,                                  /* init master */
    NULL,                                  /* init module */
    NULL,                                  /* init process */
    NULL,                                  /* init thread */
    NULL,                                  /* exit thread */
    NULL,                                  /* exit process */
    NULL,                                  /* exit master */
    NGX_MODULE_V1_PADDING
};

/**
 * https://searchcode.com/codesearch/view/77527613/
 *
 */

static void ngx_http_foo_post_handler(ngx_http_request_t *r){
	ngx_int_t rc;
	ngx_chain_t out;
	ngx_str_t  content_type = ngx_string("text/xml");
	ngx_buf_t  *buf;
	u_char *req;
	int req_size;
	size_t content_length, size;

	ngx_log_debug(NGX_LOG_DEBUG_HTTP, r->connection->log, 0, "ngx_http_foo_post_handler");

	if (r->request_body == NULL || r->request_body->bufs == NULL) {
		 ngx_http_finalize_request(r, NGX_HTTP_INTERNAL_SERVER_ERROR);
		 return;
	}

	buf = r->request_body->bufs->next ? r->request_body->bufs->next->buf : r->request_body->bufs->buf;
	if (buf->in_file) {
		ngx_log_debug(NGX_LOG_DEBUG_HTTP, r->connection->log, 0, "file buffer");
		req_size = buf->file_last - buf->file_pos;
		req      = ngx_palloc(r->pool, req_size + 1);
		if (req == NULL) {
			ngx_http_finalize_request(r, NGX_HTTP_INTERNAL_SERVER_ERROR);
			return;
		}
		size     = ngx_read_file(buf->file, req, req_size, buf->file_pos);
		ngx_log_debug(NGX_LOG_DEBUG_HTTP, r->connection->log, 0, "read %d bytes from file", size);
	} else {
		ngx_log_debug(NGX_LOG_DEBUG_HTTP, r->connection->log, 0, "memory buffer");
		req_size = buf->last - buf->pos;
		req      = buf->pos;
	}

	ngx_log_debug(NGX_LOG_DEBUG_HTTP, r->connection->log, 0, "req_size=%d, req_buf=%*s", req_size, req_size, (char *)req);

	content_length = req_size;

	out.next = NULL;
	out.buf = ngx_create_temp_buf(r->pool, content_length);
	if (out.buf == NULL) {
		ngx_http_finalize_request(r, NGX_HTTP_INTERNAL_SERVER_ERROR);
		return;
	}

	out.buf->last = ngx_cpymem(out.buf->last, req, req_size);
	out.buf->last_buf = 1;
	r->headers_out.status = NGX_HTTP_OK;
	r->headers_out.content_type = content_type;
	r->headers_out.content_length_n = content_length;

	rc = ngx_http_send_header(r);
	if (rc == NGX_OK) {
		rc = ngx_http_output_filter(r, &out);
	}

	ngx_http_finalize_request(r, rc);
}

/* Handler function */
static ngx_int_t ngx_http_echo_handler(ngx_http_request_t *r) {
//    ngx_int_t rc;
//    ngx_buf_t *b;
//    ngx_chain_t out;
//    ngx_http_echo_loc_conf_t *elcf;
//    /**
//     * 类似还提供了 ngx_http_get_module_main_conf ngx_http_get_module_srv_conf
//     */
//    elcf = ngx_http_get_module_loc_conf(r, ngx_http_echo_module);
//    if(!(r->method & (NGX_HTTP_HEAD|NGX_HTTP_GET|NGX_HTTP_POST))) {
//        return NGX_HTTP_NOT_ALLOWED;
//    }
//    r->headers_out.content_type.len = sizeof("text/html") - 1;
//    r->headers_out.content_type.data = (u_char *) "text/html";
//    r->headers_out.status = NGX_HTTP_OK;
//    r->headers_out.content_length_n = elcf->ed.len;
//    if(r->method == NGX_HTTP_HEAD)
//    {
//        rc = ngx_http_send_header(r);
//        if(rc != NGX_OK)
//        {
//            return rc;
//        }
//    }
//    b = ngx_pcalloc(r->pool, sizeof(ngx_buf_t));
//    if(b == NULL)
//    {
//        ngx_log_error(NGX_LOG_ERR, r->connection->log, 0, "Failed to allocate response buffer.");
//        return NGX_HTTP_INTERNAL_SERVER_ERROR;
//    }
//    out.buf = b;
//    out.next = NULL;
//    b->pos = elcf->ed.data;
//    b->last = elcf->ed.data + (elcf->ed.len);
//    b->memory = 1;
//    b->last_buf = 1;
//    rc = ngx_http_send_header(r);
//    if(rc != NGX_OK)
//    {
//        return rc;
//    }
//    return ngx_http_output_filter(r, &out);

	ngx_log_debug(NGX_LOG_DEBUG_HTTP, r->connection->log, 0, "to handle");

	if (!(r->method & (NGX_HTTP_HEAD|NGX_HTTP_GET|NGX_HTTP_POST))) {
		ngx_log_debug(NGX_LOG_DEBUG_HTTP, r->connection->log, 0, "error http protocal");
		return NGX_DECLINED;
	}

	r->request_body_in_single_buf = 1;
	ngx_int_t rc = ngx_http_read_client_request_body(r, ngx_http_foo_post_handler);
	if (rc == NGX_ERROR || rc >= NGX_HTTP_SPECIAL_RESPONSE) {
		return rc;
	}

	return NGX_DONE;
}

static char * ngx_http_echo(ngx_conf_t *cf, ngx_command_t *cmd, void *conf) {
    ngx_http_core_loc_conf_t  *clcf;
    clcf = ngx_http_conf_get_module_loc_conf(cf, ngx_http_core_module);
    clcf->handler = ngx_http_echo_handler;
    ngx_conf_set_str_slot(cf,cmd,conf);
    return NGX_CONF_OK;
}

static void * ngx_http_echo_create_loc_conf(ngx_conf_t *cf) {
    ngx_http_echo_loc_conf_t  *conf;
    conf = ngx_pcalloc(cf->pool, sizeof(ngx_http_echo_loc_conf_t));
    if (conf == NULL) {
        return NGX_CONF_ERROR;
    }
    conf->ed.len = 0;
    conf->ed.data = NULL;

    return conf;
}

static char * ngx_http_echo_merge_loc_conf(ngx_conf_t *cf, void *parent, void *child) {
    ngx_http_echo_loc_conf_t *prev = parent;
    ngx_http_echo_loc_conf_t *conf = child;
    ngx_conf_merge_str_value(conf->ed, prev->ed, "");

    return NGX_CONF_OK;
}
