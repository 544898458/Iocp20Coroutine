/*

	Create server/client self-signed certificate/key (self signed, DONT ADD PASSWORD)

	openssl req -x509 -newkey rsa:2048 -days 3650 -nodes -keyout client-key.pem -out client-cert.pem
	openssl req -x509 -newkey rsa:2048 -days 3650 -nodes -keyout server-key.pem -out server-cert.pem

*/
#include <glog/logging.h>
#include "SslTlsSvr.h"
#include <stdio.h>
#include <stdlib.h>

#include <openssl/err.h>
#include <openssl/dh.h>
#include <openssl/ssl.h>
#include <openssl/conf.h>
#include <openssl/engine.h>

#pragma comment(lib, "libssl.lib")
#pragma comment(lib, "libcrypto.lib")



/* SSL debug */
#define SSL_WHERE_INFO(ssl, w, flag, msg) {                \
    if(w & flag) {                                         \
      printf("+ %s: ", name);                              \
      printf("%20.20s", msg);                              \
      printf(" - %30.30s ", SSL_state_string_long(ssl));   \
      printf(" - %5.10s ", SSL_state_string(ssl));         \
      printf("	ret:%d\n", ret);                                        \
    }                                                      \
  } 

//typedef void(*info_callback)();
typedef void(*info_callback) (const SSL* ssl, int type, int val);
struct  krx {
	SSL_CTX* ctx;                                                                       /* main ssl context */
	SSL* ssl;                                                                           /* the SSL* which represents a "connection" */
	BIO* in_bio;                                                                        /* we use memory read bios */
	BIO* out_bio;                                                                       /* we use memory write bios */
	char name[512];
};

void krx_begin();                                                                     /* initialize SSL */
void krx_end();                                                                       /* shutdown SSL */
int krx_ssl_ctx_init(krx* k, const char* keyname);                                    /* initialize the SSL_CTX */
int krx_ssl_init(krx* k, int isserver, info_callback cb);                             /* init the SSL* (the "connection"). we use the `isserver` to tell SSL that it should either use the server or client protocol */
int krx_ssl_shutdown(krx* k);                                                         /* cleanup SSL allocated mem */
int krx_ssl_verify_peer(int ok, X509_STORE_CTX* ctx);                                 /* we set the SSL_VERIFY_PEER option on the SSL_CTX, so that the server will request the client certificate. We can use the certificate to get/verify the fingerprint */
int krx_ssl_handle_traffic(krx* from, krx* to);

/* some debug info */
void krx_ssl_server_info_callback(const SSL* ssl, int where, int ret);                /* purely for debug purposes; logs server info. */
void krx_ssl_client_info_callback(const SSL* ssl, int where, int ret);                /* client info callback */
void krx_ssl_info_callback(const SSL* ssl, int where, int ret, const char* name);     /* generic info callback */

int main2() {

	/* startup SSL */
	krx_begin();

	/* create client/server objects */
	krx server;
	krx client;

	/* init server. */
	if (krx_ssl_ctx_init(&server, "server") < 0) {
		exit(EXIT_FAILURE);
	}
	if (krx_ssl_init(&server, 1, krx_ssl_server_info_callback) < 0) {
		exit(EXIT_FAILURE);
	}

	printf("+ Initialized server.\n");

	/* init client. */
	if (krx_ssl_ctx_init(&client, "client") < 0) {
		exit(EXIT_FAILURE);
	}
	if (krx_ssl_init(&client, 0, krx_ssl_client_info_callback) < 0) {
		exit(EXIT_FAILURE);
	}

	printf("+ Initialized client.\n");

	/* kickoff handshake; initiated by client (e.g. browser) */
	SSL_do_handshake(client.ssl);
	krx_ssl_handle_traffic(&client, &server);
	krx_ssl_handle_traffic(&server, &client);
	krx_ssl_handle_traffic(&client, &server);
	krx_ssl_handle_traffic(&server, &client);

	/* encrypt some data and send it to the client */
	char buf[521] = { 0 };
	sprintf(buf, "%s", "Hello world");
	SSL_write(server.ssl, buf, sizeof(buf));
	krx_ssl_handle_traffic(&server, &client);

	krx_ssl_shutdown(&server);
	krx_ssl_shutdown(&client);

	krx_end();
	return EXIT_SUCCESS;
}

void krx_begin() {
	SSL_library_init();
	SSL_load_error_strings();
	//ERR_load_BIO_strings();
	OpenSSL_add_all_algorithms();
	OpenSSL_add_ssl_algorithms();
}

void krx_end() {
	//ERR_remove_state(0);
	ENGINE_cleanup();
	//CONF_modules_unload(1);
	ERR_free_strings();
	EVP_cleanup();
	sk_SSL_COMP_free(SSL_COMP_get_compression_methods());
	CRYPTO_cleanup_all_ex_data();
}

int krx_ssl_ctx_init(krx* k, const char* keyname) {

	int r = 0;

	/* create a new context using DTLS */
	k->ctx = SSL_CTX_new(TLS_server_method());
	if (!k->ctx) {
		printf("Error: cannot create SSL_CTX.\n");
		ERR_print_errors_fp(stderr);
		return -1;
	}

	/* set our supported ciphers */
	r = SSL_CTX_set_cipher_list(k->ctx, "ALL:!ADH:!LOW:!EXP:!MD5:@STRENGTH");
	if (r != 1) {
		printf("Error: cannot set the cipher list.\n");
		ERR_print_errors_fp(stderr);
		return -2;
	}

	/* the client doesn't have to send it's certificate */
	//SSL_CTX_set_verify(k->ctx, SSL_VERIFY_PEER, krx_ssl_verify_peer);
	/* 服务器不验证客户端证书， 客户端可以不提供证书*/
	//SSL_CTX_set_verify(k->ctx, SSL_VERIFY_NONE, NULL);

	/* enable srtp */
	r = SSL_CTX_set_tlsext_use_srtp(k->ctx, "SRTP_AES128_CM_SHA1_80");
	if (r != 0) {
		printf("Error: cannot setup srtp.\n");
		ERR_print_errors_fp(stderr);
		return -3;
	}

	/* load key and certificate */
	char certfile[1024];
	char keyfile[1024];
	sprintf(certfile, "./%s-cert.crt", keyname);
	sprintf(keyfile, "./%s-private.key", keyname);

	/* certificate file; contains also the public key */
	r = SSL_CTX_use_certificate_file(k->ctx, certfile, SSL_FILETYPE_PEM);
	if (r != 1) {
		LOG(ERROR) << "Error: cannot load certificate file.";
		ERR_print_errors_fp(stderr);
		return -4;
	}

	/* load private key */
	r = SSL_CTX_use_PrivateKey_file(k->ctx, keyfile, SSL_FILETYPE_PEM);
	if (r != 1) {
		printf("Error: cannot load private key file.\n");
		ERR_print_errors_fp(stderr);
		return -5;
	}

	/* check if the private key is valid */
	r = SSL_CTX_check_private_key(k->ctx);
	if (r != 1) {
		printf("Error: checking the private key failed. \n");
		ERR_print_errors_fp(stderr);
		return -6;
	}

	sprintf(k->name, "+ %s", keyname);

	return 0;
}

int krx_ssl_verify_peer(int ok, X509_STORE_CTX* ctx) {
	return 1;
}

const char OPENSSL_LOG_DEB[] = "OPENSSL_LOG_DEB";
static void openssl_msg_cb(int write_p, int version, int content_type, const void* buf, size_t len, SSL* ssl, void* arg)
{
	const char* str_write_p, * str_version, * str_content_type =
		"", * str_details1 = "", * str_details2 = "";

	char msg_buffer[65535] = { 0 };
	int msg_len = 0;

	str_write_p = write_p ? ">>>" : "<<<";

	switch (version) {
	case SSL2_VERSION:
		str_version = "SSL 2.0";
		break;
	case SSL3_VERSION:
		str_version = "SSL 3.0 ";
		break;
	case TLS1_VERSION:
		str_version = "TLS 1.0 ";
		break;
	case TLS1_1_VERSION:
		str_version = "TLS 1.1 ";
		break;
	case TLS1_2_VERSION:
		str_version = "TLS 1.2 ";
		break;
	case DTLS1_VERSION:
		str_version = "DTLS 1.0 ";
		break;
	case DTLS1_BAD_VER:
		str_version = "DTLS 1.0 (bad) ";
		break;
	default:
		str_version = "???";
	}

	if (version == SSL2_VERSION) {
		str_details1 = "???";

		if (len > 0) {
			switch (((const unsigned char*)buf)[0]) {
			case 0:
				str_details1 = ", ERROR:";
				str_details2 = " ???";
				if (len >= 3) {
					unsigned err =
						(((const unsigned char*)buf)[1] << 8) +
						((const unsigned char*)buf)[2];

					switch (err) {
					case 0x0001:
						str_details2 = " NO-CIPHER-ERROR";
						break;
					case 0x0002:
						str_details2 = " NO-CERTIFICATE-ERROR";
						break;
					case 0x0004:
						str_details2 = " BAD-CERTIFICATE-ERROR";
						break;
					case 0x0006:
						str_details2 = " UNSUPPORTED-CERTIFICATE-TYPE-ERROR";
						break;
					}
				}

				break;
			case 1:
				str_details1 = ", CLIENT-HELLO";
				break;
			case 2:
				str_details1 = ", CLIENT-MASTER-KEY";
				break;
			case 3:
				str_details1 = ", CLIENT-FINISHED";
				break;
			case 4:
				str_details1 = ", SERVER-HELLO";
				break;
			case 5:
				str_details1 = ", SERVER-VERIFY";
				break;
			case 6:
				str_details1 = ", SERVER-FINISHED";
				break;
			case 7:
				str_details1 = ", REQUEST-CERTIFICATE";
				break;
			case 8:
				str_details1 = ", CLIENT-CERTIFICATE";
				break;
			}
		}
	}

	if (version == SSL3_VERSION ||
		version == TLS1_VERSION ||
		version == TLS1_1_VERSION ||
		version == TLS1_2_VERSION ||
		version == DTLS1_VERSION ||
		version == DTLS1_2_VERSION ||
		version == DTLS1_BAD_VER) {
		switch (content_type) {
		case 20:
			str_content_type = "ChangeCipherSpec";
			break;
		case 21:
			str_content_type = "Alert";
			break;
		case 22:
			str_content_type = "Handshake";
			break;
		}

		if (content_type == 21) { /* Alert */
			str_details1 = ", ???";

			if (len == 2) {
				switch (((const unsigned char*)buf)[0]) {
				case 1:
					str_details1 = ", warning";
					break;
				case 2:
					str_details1 = ", fatal";
					break;
				}

				str_details2 = " ???";
				switch (((const unsigned char*)buf)[1]) {
				case 0:
					str_details2 = " close_notify";
					break;
				case 10:
					str_details2 = " unexpected_message";
					break;
				case 20:
					str_details2 = " bad_record_mac";
					break;
				case 21:
					str_details2 = " decryption_failed";
					break;
				case 22:
					str_details2 = " record_overflow";
					break;
				case 30:
					str_details2 = " decompression_failure";
					break;
				case 40:
					str_details2 = " handshake_failure";
					break;
				case 42:
					str_details2 = " bad_certificate";
					break;
				case 43:
					str_details2 = " unsupported_certificate";
					break;
				case 44:
					str_details2 = " certificate_revoked";
					break;
				case 45:
					str_details2 = " certificate_expired";
					break;
				case 46:
					str_details2 = " certificate_unknown";
					break;
				case 47:
					str_details2 = " illegal_parameter";
					break;
				case 48:
					str_details2 = " unknown_ca";
					break;
				case 49:
					str_details2 = " access_denied";
					break;
				case 50:
					str_details2 = " decode_error";
					break;
				case 51:
					str_details2 = " decrypt_error";
					break;
				case 60:
					str_details2 = " export_restriction";
					break;
				case 70:
					str_details2 = " protocol_version";
					break;
				case 71:
					str_details2 = " insufficient_security";
					break;
				case 80:
					str_details2 = " internal_error";
					break;
				case 90:
					str_details2 = " user_canceled";
					break;
				case 100:
					str_details2 = " no_renegotiation";
					break;
				case 110:
					str_details2 = " unsupported_extension";
					break;
				case 111:
					str_details2 = " certificate_unobtainable";
					break;
				case 112:
					str_details2 = " unrecognized_name";
					break;
				case 113:
					str_details2 = " bad_certificate_status_response";
					break;
				case 114:
					str_details2 = " bad_certificate_hash_value";
					break;
				case 115:
					str_details2 = " unknown_psk_identity";
					break;
				}
			}
		}

		if (content_type == 22) { /* Handshake */
			str_details1 = "???";

			if (len > 0) {
				switch (((const unsigned char*)buf)[0]) {
				case 0:
					str_details1 = ", HelloRequest";
					break;
				case 1:
					str_details1 = ", ClientHello";
					break;
				case 2:
					str_details1 = ", ServerHello";
					break;
				case 3:
					str_details1 = ", HelloVerifyRequest";
					break;
				case 11:
					str_details1 = ", Certificate";
					break;
				case 12:
					str_details1 = ", ServerKeyExchange";
					break;
				case 13:
					str_details1 = ", CertificateRequest";
					break;
				case 14:
					str_details1 = ", ServerHelloDone";
					break;
				case 15:
					str_details1 = ", CertificateVerify";
					break;
				case 16:
					str_details1 = ", ClientKeyExchange";
					break;
				case 20:
					str_details1 = ", Finished";
					break;
				}
			}
		}

#ifndef OPENSSL_NO_HEARTBEATS
		if (content_type == 24) { /* Heartbeat */
			str_details1 = ", Heartbeat";

			if (len > 0) {
				switch (((const unsigned char*)buf)[0]) {
				case 1:
					str_details1 = ", HeartbeatRequest";
					break;
				case 2:
					str_details1 = ", HeartbeatResponse";
					break;
				}
			}
		}
#endif
	}

	msg_len += snprintf(msg_buffer + msg_len, sizeof(msg_buffer) - msg_len - 1,
		"%s %s%s [length %04lx]%s%s\n", str_write_p, str_version,
		str_content_type, (unsigned long)len, str_details1, str_details2);

	if (len > 0) {
		size_t num, i;
		msg_len += snprintf(msg_buffer + msg_len, sizeof(msg_buffer) - msg_len - 1, "%s", "   ");
		num = len;

		for (i = 0; i < num; i++) {
			if (i % 32 == 0 && i > 0) {
				msg_len += snprintf(msg_buffer + msg_len, sizeof(msg_buffer) - msg_len - 1, "%s", "\n   ");
			}
			msg_len += snprintf(msg_buffer + msg_len, sizeof(msg_buffer) - msg_len - 1, " %02x", ((const unsigned char*)buf)[i]);
		}

		if (i < len) {

			msg_len += snprintf(msg_buffer + msg_len, sizeof(msg_buffer) - msg_len - 1, "%s", " ...");
		}

		msg_len += snprintf(msg_buffer + msg_len, sizeof(msg_buffer) - msg_len - 1, "%s", "\n");
	}

	LOG(INFO) << OPENSSL_LOG_DEB << "," << msg_buffer;
}
static void openssl_info_callback(const SSL* s, int where, int ret)
{
	const char* str;
	int w;

	w = where & ~SSL_ST_MASK;

	if (w & SSL_ST_CONNECT) {
		str = "SSL_connect";
	}
	else if (w & SSL_ST_ACCEPT) {
		str = "SSL_accept";
	}
	else {
		str = "undefined";
	}

	if (where & SSL_CB_LOOP) {
		LOG(INFO) << "SSL_LOG_LOOP," << str << ": " << SSL_state_string_long(s);
	}
	else if (where & SSL_CB_ALERT) {
		str = (where & SSL_CB_READ) ? "read" : "write";
		LOG(WARNING) << "SSL_LOG_ALERT," << "SSL3 alert,str:" << str << ",Type:" << SSL_alert_type_string_long(ret) << "," << SSL_alert_desc_string_long(ret);
	}
	else if (where & SSL_CB_EXIT) {
		if (ret == 0) {
			LOG(ERROR) << "SSL_LOG_EXIT, " << str << ":failed in, " << SSL_state_string_long(s);
		}
		else if (ret < 0) {
			LOG(INFO) << ret <<"=ret,str =" << str << ",error in, " << SSL_state_string_long(s);
		}
	}
}

/* this sets up the SSL* */
int krx_ssl_init(krx* k, int isserver, info_callback cb) {

	/* create SSL* */
	k->ssl = SSL_new(k->ctx);
	if (!k->ssl) {
		printf("Error: cannot create new SSL*.\n");
		return -1;
	}

	/* info callback */
	SSL_set_info_callback(k->ssl, openssl_info_callback);
	SSL_set_msg_callback(k->ssl, openssl_msg_cb);
	SSL_set_msg_callback_arg(k->ssl, NULL);


	/* bios */
	k->in_bio = BIO_new(BIO_s_mem());
	if (k->in_bio == NULL) {
		printf("Error: cannot allocate read bio.\n");
		return -2;
	}

	BIO_set_mem_eof_return(k->in_bio, -1); /* see: https://www.openssl.org/docs/crypto/BIO_s_mem.html */

	k->out_bio = BIO_new(BIO_s_mem());
	if (k->out_bio == NULL) {
		printf("Error: cannot allocate write bio.\n");
		return -3;
	}

	BIO_set_mem_eof_return(k->out_bio, -1); /* see: https://www.openssl.org/docs/crypto/BIO_s_mem.html */

	SSL_set_bio(k->ssl, k->in_bio, k->out_bio);

	/* either use the server or client part of the protocol */
	if (isserver == 1) {
		SSL_set_accept_state(k->ssl);
	}
	else {
		SSL_set_connect_state(k->ssl);
	}

	return 0;
}

void krx_ssl_server_info_callback(const SSL* ssl, int where, int ret) {
	krx_ssl_info_callback(ssl, where, ret, "server");
}
void krx_ssl_client_info_callback(const SSL* ssl, int where, int ret) {
	krx_ssl_info_callback(ssl, where, ret, "client");
}

void krx_ssl_info_callback(const SSL* ssl, int where, int ret, const char* name) {

	if (ret == 0) {
		printf("-- krx_ssl_info_callback: error occured.\n");
		return;
	}

	SSL_WHERE_INFO(ssl, where, SSL_CB_LOOP, "LOOP");
	SSL_WHERE_INFO(ssl, where, SSL_CB_HANDSHAKE_START, "HANDSHAKE START");
	SSL_WHERE_INFO(ssl, where, SSL_CB_HANDSHAKE_DONE, "HANDSHAKE DONE");
	SSL_WHERE_INFO(ssl, where, SSL_CB_ALERT, "SSL_CB_ALERT");
	SSL_WHERE_INFO(ssl, where, SSL_CB_EXIT, "SSL_CB_EXIT");
	SSL_WHERE_INFO(ssl, where, SSL_CB_READ, "SSL_CB_READ");
	SSL_WHERE_INFO(ssl, where, SSL_CB_WRITE, "SSL_CB_WRITE");
	SSL_WHERE_INFO(ssl, where, SSL_ST_ACCEPT, "SSL_ST_ACCEPT");
	SSL_WHERE_INFO(ssl, where, SSL_ST_CONNECT, "SSL_ST_CONNECT");
}

int krx_ssl_handle_traffic(krx* from, krx* to) {

	// Did SSL write something into the out buffer
	char outbuf[4096];
	int written = 0;
	int read = 0;
	auto pending = BIO_ctrl_pending(from->out_bio); //使用BIO_ctrl_pending()检查输出bio中存储了多少字节。请参阅krx_ssl_handle_traffic()本文底部的代码清单。

	if (pending > 0) {
		read = BIO_read(from->out_bio, outbuf, sizeof(outbuf));
	}
	printf("%s Pending %d, and read: %d\n", from->name, (int)pending, read);

	if (read > 0) {
		written = BIO_write(to->in_bio, outbuf, read);
	}

	if (written > 0) {
		if (!SSL_is_init_finished(to->ssl)) {
			SSL_do_handshake(to->ssl);
		}
		else {
			read = SSL_read(to->ssl, outbuf, sizeof(outbuf));
			printf("%s read: %s\n", to->name, outbuf);
		}
	}

	return 0;
}

int krx_ssl_shutdown(krx* k) {
	if (!k) {
		return -1;
	}

	if (k->ctx) {
		SSL_CTX_free(k->ctx);
		k->ctx = NULL;
	}

	if (k->ssl) {
		SSL_free(k->ssl);
		k->ssl = NULL;
	}

	return 0;
}

SslTlsSvr::SslTlsSvr() :m_pServer(new krx)
{
}

SslTlsSvr::~SslTlsSvr()
{
	delete m_pServer;
	m_pServer = nullptr;
}

void SslTlsSvr::InitAll()
{
	krx_begin();
}
void SslTlsSvr::Init(const bool bServer)
{
	/* init server. */
	if (krx_ssl_ctx_init(m_pServer, bServer ? "server" : "client") < 0) {
		exit(EXIT_FAILURE);
	}
	if (krx_ssl_init(m_pServer, bServer, bServer ? krx_ssl_server_info_callback : krx_ssl_client_info_callback) < 0) {
		exit(EXIT_FAILURE);
	}
}

SslTlsSvr g_Test;
int SslTlsSvr::处理前端发来的密文(const void* buf, const int len)
{
	//g_Test.Init(false);
	//g_Test.do_handshake();
	//char buf密文[2048];
	//const auto len密文 = g_Test.获取准备发往前端的密文(buf密文);
	const int32_t i32已处理密文字节 = BIO_write(m_pServer->in_bio, buf, len);
	//const int32_t i32已处理密文字节 = BIO_write(m_pServer->in_bio, buf密文, len密文);
	if (i32已处理密文字节 > 0)
	{
		if (!SSL_is_init_finished(m_pServer->ssl))//还没握手，要回应握手
		{
			SSL_do_handshake(m_pServer->ssl);
		}
		//else
		//{
		//	char buf明文[2048];
		//	const auto read明文 = SSL_read(m_pServer->ssl, buf明文, sizeof(buf明文));
		//	LOG(WARNING) << m_pServer->name << " read: " << read明文;
		//}
	}
	return i32已处理密文字节;
}

int SslTlsSvr::把要发给前端的明文交给Ssl处理(const void* buf, const int len)
{
	return SSL_write(m_pServer->ssl, buf, len);
}

void SslTlsSvr::do_handshake()
{
	SSL_do_handshake(m_pServer->ssl);
}


template<int len>
int SslTlsSvr::读出已解密的明文(char(&bufOut)[len])
{
	const int read = SSL_read(m_pServer->ssl, bufOut, sizeof(bufOut));
	LOG(INFO) << m_pServer->name << " read: " << read;
	return read;
}

template<int len>
int SslTlsSvr::获取准备发往前端的密文(char(&bufOut)[len])
{
	int pending = BIO_ctrl_pending(m_pServer->out_bio); //使用BIO_ctrl_pending()检查输出bio中存储了多少字节。请参阅krx_ssl_handle_traffic()本文底部的代码清单。
	if (pending <= 0)
		return 0;

	const auto read = BIO_read(m_pServer->out_bio, bufOut, sizeof(bufOut));
	LOG(INFO) << "pending " << pending << ",read " << read;
	return read;
}

template int SslTlsSvr::获取准备发往前端的密文(char(&bufOut)[4096]);
template int SslTlsSvr::读出已解密的明文(char(&bufOut)[4096]);