#include <future>
#include <chrono>
#include <csound.hpp>
#include <OpcodeBase.hpp>
#include <cstdio>
#include <ctime>
#include <iostream>
#include <queue>
#include <thread>
#include <map>
#include <memory>
#include <cpp-httplib/httplib.h>
#include <jsonrpccxx/server.hpp>

namespace csound_webserver {
    
    class CsoundWebServer;
    
    typedef csound::heap_object_manager_t<CsoundWebServer> webservers;
    
    static std::mutex &get_mutex() {
        static std::mutex mutex_;
        return mutex_;
    }
    
    static bool diagnostics_enabled = true;

/*  To implement:   

    CompileCsdText
    CompileOrc
    EvalCode
    Get0dBFS
    GetAudioChannel
    GetControlChannel
    GetDebug
    GetKsmps
    GetNchnls
    GetNchnlsInput
    GetScoreOffsetSeconds
    GetScoreTime
    GetSr
    GetStringChannel
    InputMessage
    IsScorePending
    Message (this is the only asynchronous method)
    ReadScore
    RewindScore
    ScoreEvent
    SetControlChannel
    SetDebug
    SetMessageCallback
    SetScoreOffsetSeconds
    SetScorePending
    SetStringChannel
    TableGet
    TableLength
    TableSet    
    
*/

    struct CsoundWebServer {
        // All resources are served relative to the server's base directory.
        std::string base_directory;
        // All resources are identified by appending their path to the 
        // origin.
        std::string origin;
        int port;
        httplib::Server server;
        std::thread *listener_thread;
        CSOUND *csound;
        CsoundWebServer() {
            if (diagnostics_enabled) std::fprintf(stderr, "CsoundWebServer::CsoundWebServer...\n");
            if (diagnostics_enabled) std::fprintf(stderr, "CsoundWebServer::CsoundWebServer.\n");
        }
        virtual ~CsoundWebServer() {
            if (diagnostics_enabled) std::fprintf(stderr, "CsoundWebServer::~CsoundWebServer...\n");
            server.stop();
            if (diagnostics_enabled) std::fprintf(stderr, "CsoundWebServer::~CsoundWebServer.\n");
        }
        virtual void listen() {
            if (diagnostics_enabled) std::fprintf(stderr, "CsoundWebServer::listen...\n");
            // Was "0.0.0.0" for all interfaces -- for security we limit this 
            // to localhost.
            server.listen("localhost", port);
            if (diagnostics_enabled) std::fprintf(stderr, "CsoundWebServer::listen.\n");
        }
        virtual void create_(CSOUND *csound_, const std::string &base_directory_, int port_) {
            csound = csound_;
            base_directory = base_directory_;
            if(port != -1) {
                port = port_;
            } else {
                port = 8080;
            }
            if (diagnostics_enabled) {
                server.set_logger([] (const auto& req, const auto& res) {
                    std::fprintf(stderr, "Request:  method: %s path: %s body: %s\n", req.method.c_str(), req.path.c_str(), req.body.c_str());
                    std::fprintf(stderr, "Response: reason: %s body: %s\n", res.reason.c_str(), res.body.c_str());
                });
            }
            server.set_base_dir(base_directory.c_str());
            if (diagnostics_enabled) std::fprintf(stderr, "CsoundWebServer::create: base_directory: %s\n", base_directory.c_str());
            origin = "http://localhost:" + std::to_string(port);
            csound->Message(csound_, "CsoundWebServer: origin: %s\n", origin.c_str());
            // Add JSON-RPC skeletons...
            server.Post("/CompileCsdText", [&](const httplib::Request &request, httplib::Response &response) {
                int result = OK;
                std::fprintf(stderr, "/CompileCsdText...\n");
                //result = csound->CompileCsdText(csound, "boo");
                response.set_content(std::to_string(result), "text/plain");
                response.status = 201;
            });
            // ...and start listening in a separate thread.
            listener_thread = new std::thread(&CsoundWebServer::listen, this);
        }
        static CsoundWebServer *create(CSOUND *csound_, const std::string &base_directory_, int port_) {
            if (diagnostics_enabled) std::fprintf(stderr, "CsoundWebServer::create...\n");
            auto webserver = new CsoundWebServer();
            webserver->create_(csound_, base_directory_, port_);
            if (diagnostics_enabled) std::fprintf(stderr, "CsoundWebServer::create.\n");
            return webserver;
        }
        virtual void open_resource(const std::string &resource, const std::string &browser) {          
            if (diagnostics_enabled) std::fprintf(stderr, "CsoundWebServer::open_resource...\n");
            // Origin:
            // `http://host:port`
            // Complete URL:            
            // `http://host:port[[/resource_path][.extension]]`
            // NOTE: The _filesystem_ base directory is mapped to: `origin/`.
            std::string url;
            if (resource.find("http://") == 0) {
                url = resource;
            } else if (resource.find("https://") == 0) {
                url = resource;
            } else {
                url = origin + "/" + resource;
            }
            std::string command = browser + " " + url;
            std::system(command.c_str());
            if (diagnostics_enabled) std::fprintf(stderr, "CsoundWebServer::open_resource: command: %s\n", command.c_str());
        }
        virtual void open_html(const std::string &html_text, const std::string &browser) {
            if (diagnostics_enabled) std::fprintf(stderr, "CsoundWebServer::open_html...\n");
            // Origin:
            // `http://host:port`
            // Complete URL:            
            // `http://host:port[[/resource_path][.extension]]`
            // NOTE: The _filesystem_ base directory is mapped to: `origin/`.
            // Creaate a temporary .html file from the html text in the base directory.
            char html_path[0x500];
            {
                std::lock_guard lock(get_mutex());
                std::mt19937 mersenne_twister;
                unsigned int seed_ = std::time(nullptr);
                mersenne_twister.seed(seed_);
                std::snprintf(html_path, 0x500, "csound_webserver_%x.html", mersenne_twister());
                std::string filepath = base_directory + "/" + html_path;
                auto file_ = fopen(filepath.c_str(), "w+");
                std::fwrite(html_text.c_str(), html_text.length(), sizeof(html_text[0]), file_);
                std::fclose(file_);
            }
            std::string url = origin + "/" + html_path;
            std::string command = browser + " " + url;
            if (diagnostics_enabled) std::fprintf(stderr, "CsoundWebServer::html_text: command: %s\n", command.c_str());
            std::system(command.c_str());
            if (diagnostics_enabled) std::fprintf(stderr, "CsoundWebServer::html_text.\n");
        }
    };

    class csound_webserver_create : public csound::OpcodeBase<csound_webserver_create> {
        public:
            // OUTPUTS
            MYFLT *i_server_handle;
            // INPUTS
            STRINGDAT *S_base_uri;
            MYFLT *i_port;
            MYFLT *i_diagnostics_enabled;
            int init(CSOUND *csound) {
                int result = OK;
                std::string base_uri_ = S_base_uri->data;
                int port = *i_port;
                diagnostics_enabled = *i_diagnostics_enabled;
                auto server = CsoundWebServer::create(csound, base_uri_, port);
                int handle = webservers::instance().handle_for_object(csound, server);
                *i_server_handle = static_cast<MYFLT>(handle);
                return result;
            }
    };

    class csound_webserver_open_resource : public csound::OpcodeBase<csound_webserver_open_resource> {
        public:
            // OUTPUTS
            // INPUTS
            MYFLT *i_server_handle_;
            STRINGDAT *S_resource;
            STRINGDAT *S_browser;
            // STATE
            CsoundWebServer *server;
            int init(CSOUND *csound) {
                log(csound, "csound_webserver_open_resource::init: this: %p\n", this);
                int result = OK;
                int i_server_handle = *i_server_handle_;
                std::string resource = S_resource->data;
                std::string browser = S_browser->data;
                server = webservers::instance().object_for_handle(csound, i_server_handle);
                server->open_resource(resource, browser);
                log(csound, "csound_webserver_open_resource::init.\n");
                return result;
            }
            int kontrol(CSOUND *csound) {
                int result = OK;
                ///server->handle_events();
                return OK;
            }
    };

    class csound_webserver_open_html : public csound::OpcodeBase<csound_webserver_open_html> {
        public:
            // OUTPUTS
            // INPUTS
            MYFLT *i_server_handle_;
            STRINGDAT *S_html_text_;
            STRINGDAT *S_browser;
            // STATE
            CsoundWebServer *server;
            int init(CSOUND *csound) {
                log(csound, "csound_webserver_open_html::init: this: %p\n", this);
                int result = OK;
                int i_server_handle = *i_server_handle_;
                std::string html_text = S_html_text_->data;
                std::string browser = S_browser->data;
                server = webservers::instance().object_for_handle(csound, i_server_handle);
                server->open_html(html_text, browser);
                log(csound, "csound_webserver_open_html::init.\n");
                return result;
            }
            int kontrol(CSOUND *csound) {
                int result = OK;
                ///server->handle_events();
                return OK;
            }
    };

};

/**
 * i_webserver_handle webserver_create S_base_uri, i_port [, i_diagnostics_enabled]
 * webserver_open_resource i_webserver_handle, S_resource [, S_browser_command]
 * webserver_open_html i_webserver_handle, S_html_text [, S_browser_command]
 */
extern "C" {
    
    PUBLIC int csoundModuleInit_csound_webserver(CSOUND *csound) {
        std::fprintf(stderr, "csoundModuleInit_csound_webserver...\n");
        int status = csound->AppendOpcode(csound,
                (char *)"webserver_create",
                sizeof(csound_webserver::csound_webserver_create),
                0,
                1,
                (char *)"i",
                (char *)"Sio",
                (int (*)(CSOUND*,void*)) csound_webserver::csound_webserver_create::init_,
                (int (*)(CSOUND*,void*)) 0,
                (int (*)(CSOUND*,void*)) 0);
        status += csound->AppendOpcode(csound,
                (char *)"webserver_open_resource",
                sizeof(csound_webserver::csound_webserver_open_resource),
                0,
                3,
                (char *)"",
                (char *)"iSS",
                (int (*)(CSOUND*,void*)) csound_webserver::csound_webserver_open_resource::init_,
                (int (*)(CSOUND*,void*)) csound_webserver::csound_webserver_open_resource::kontrol_,
                (int (*)(CSOUND*,void*)) 0);
        status += csound->AppendOpcode(csound,
                (char *)"webserver_open_html",
                sizeof(csound_webserver::csound_webserver_open_html),
                0,
                3,
                (char *)"",
                (char *)"iSS",
                (int (*)(CSOUND*,void*)) csound_webserver::csound_webserver_open_html::init_,
                (int (*)(CSOUND*,void*)) csound_webserver::csound_webserver_open_html::kontrol_,
                (int (*)(CSOUND*,void*)) 0);
        return status;
    }

    PUBLIC int csoundModuleDestroy_csound_webserver(CSOUND *csound) {
        csound_webserver::webservers::instance().module_destroy(csound);
        return OK;
    }

#ifndef INIT_STATIC_MODULES
    PUBLIC int csoundModuleCreate(CSOUND *csound) {
        return OK;
    }

    PUBLIC int csoundModuleInit(CSOUND *csound) {
        return csoundModuleInit_csound_webserver(csound);
    }

    PUBLIC int csoundModuleDestroy(CSOUND *csound) {
        return csoundModuleDestroy_csound_webserver(csound);
    }
#endif
}

