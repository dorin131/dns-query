#include <iostream>
#include <fstream>
#include <sstream>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

class DNS
{
  public:
    DNS();
    void get(std::string);
  private:
    const std::string DNS_SERVER = "1.1.1.1";
    const int PORT = 53;

    // Will contain header and question
    unsigned char msg_buffer[60000];

    /**
     * Message header
     * (https://www.freesoft.org/CIE/RFC/1035/40.htm)
     **/
    typedef struct {
      // Transaction ID
      unsigned short id;

      // Flags
      unsigned char RecursionDesired : 1;
      unsigned char Truncation : 1;
      unsigned char Authoritative : 1;
      unsigned char Opcode : 4;
      unsigned char IsResponse : 1;

      unsigned char ResponseCode : 4;
      unsigned char CheckingDisabled : 1;
      unsigned char AuthenticatedData : 1;
      unsigned char Reserved : 1;
      unsigned char RecursionAvailable : 1;

      unsigned short Questions;
      unsigned short AnswerRRs;
      unsigned short AuthorityRRs;
      unsigned short AdditionalRRs;
    } DNS_header;
    DNS_header* dns_header;

    /**
     * Question
     * (https://www.freesoft.org/CIE/RFC/1035/41.htm)
     **/
    unsigned char* qname;
    typedef struct {
      unsigned short qtype;
      unsigned short qclass;
    } QData;
    QData *qdata;

    /**
     * Resource record
     * (https://www.freesoft.org/CIE/RFC/1035/42.htm)
     **/
    unsigned char* rr_name;
    typedef struct {
      unsigned short rr_type;
      unsigned short rr_class;
      unsigned int rr_ttl;
      unsigned short rr_rdlength;
      // RDATA
    } ResourceRecord;

    ResourceRecord answers[5];

    // RDATA: An IPv6 with 4 octets
    typedef unsigned char IP[4];

    std::string url_to_qname(std::string&);
    std::string qname_to_url(unsigned char*);
    std::streamsize send_dns_request();
    void build_message(std::string);
    std::string read_ip(ResourceRecord*);
};
