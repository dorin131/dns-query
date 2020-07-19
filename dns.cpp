#include "dns.h"

DNS::DNS()
{
  std::cout << "DNS Server: " << DNS_SERVER << "\n\n";
}

void DNS::get(std::string url)
{
  build_message(url);

  std::streamsize resp_sz = send_dns_request();

  auto dns_response_header = reinterpret_cast<DNS_header*>(msg_buffer);
  auto qname = &msg_buffer[sizeof(DNS_header)];
  std::cout
    << "- Query\n"
    << "Name: "
    << qname_to_url(qname) << std::endl
    << "Answers RRs: "
    << ntohs(dns_response_header->AnswerRRs) << std::endl
    << "Authority RRs: "
    << ntohs(dns_response_header->AuthorityRRs) << std::endl
    << "Additional RRs: "
    << ntohs(dns_response_header->AdditionalRRs) << std::endl;

  // Pointer to first answer
  unsigned char* answer_ptr = reinterpret_cast<unsigned char*>(
    &msg_buffer[sizeof(DNS_header) +
    strlen(reinterpret_cast<char*>(qname)) + 1 +
    sizeof(QData)]);

  // Looping through all answers
  for (int i = 0, count = ntohs(dns_response_header->AnswerRRs); i < count; i++) {
    ResourceRecord* answer = reinterpret_cast<ResourceRecord*>(
      reinterpret_cast<char*>(answer_ptr) +
      strlen(reinterpret_cast<char*>(qname)) + 1);
    std::cout
      << "\n- Answer(" << i + 1 << ")\n"
      << "Type: " << ntohs(answer->rr_type) << std::endl
      << "Class: " << ntohs(answer->rr_class) << std::endl
      << "TTL: " << ntohl(answer->rr_ttl) << std::endl
      << "Data length: " << ntohs(answer->rr_rdlength) << std::endl
      << "Address: " << read_ip(answer) << std::endl;

    // Set pointer to next answer
    if (i < count - 1) {
      answer_ptr += sizeof(ResourceRecord) + sizeof(IP);
    }
  }
}

std::string DNS::read_ip(ResourceRecord* rr)
{
  std::stringstream res;
  IP* ip = reinterpret_cast<IP*>((unsigned char*)rr + sizeof(rr) + 2);
  
  for (int i = 0; i < 4; i++) {
    if (i != 0) res << ".";
    res << static_cast<int>((*ip)[i]);
  }
  return res.str();
}

/**
 * Creating a socket and sending the contents of msg_buffer to it.
 * Then reading the response back into msg_buffer.
 * Returns size of the recived data.
 **/
std::streamsize DNS::send_dns_request()
{
  int socket_desc;
  struct sockaddr_in socket_addr;
  unsigned long socket_length;

  socket_desc = socket(AF_INET, SOCK_DGRAM, 0);
  if (socket_desc == -1) {
    std::cerr << "Failed to create socket\n";
    exit(1);
  }

  socket_addr.sin_family = AF_INET;
  socket_addr.sin_port = htons(PORT);
  socket_addr.sin_addr.s_addr = inet_addr(DNS_SERVER.c_str());

  auto send_sz = sendto(
    socket_desc,
    msg_buffer,
    sizeof(DNS_header) + strlen(reinterpret_cast<char*>(qname)) + 1 + sizeof(QData),
    0,
    reinterpret_cast<struct sockaddr*>(&socket_addr),
    sizeof(socket_addr)
  );
  if (send_sz == -1) {
    std::cerr << "Error sending message to socket\n";
    close(socket_desc);
    exit(1);
  }

  socket_length = sizeof(socket_addr);
  auto recv_sz = recvfrom(
    socket_desc,
    msg_buffer,
    60000,
    0,
    reinterpret_cast<struct sockaddr*>(&socket_addr),
    reinterpret_cast<socklen_t*>(&socket_addr)
  );
  if (recv_sz == -1) {
    std::cerr << "Error reading from socket\n";
    close(socket_desc);
    exit(1);
  }

  // std::cout.write((const char*)msg_buffer, recv_sz);

  close(socket_desc);

  return recv_sz;
}

/**
 * Adding data to dns_header, qname and qdata
 * then placing them into msg_buffer
 **/
void DNS::build_message(std::string url)
{
  dns_header = reinterpret_cast<DNS_header*>(&msg_buffer);
  dns_header->id = htons(123);
  dns_header->Questions = htons(1);
  dns_header->RecursionDesired = 1;

  qname = &msg_buffer[sizeof(DNS_header)];
  std::string qname_str = url_to_qname(url);
  memcpy(qname, qname_str.c_str(), sizeof(qname_str));

  qdata = reinterpret_cast<QData*>(
    &msg_buffer[sizeof(DNS_header) +
    strlen(reinterpret_cast<char*>(qname)) + 1]);
  qdata->qtype = htons(1);
  qdata->qclass = htons(1);
}

/**
 * QNAME is a domain-name represented as a sequence of labels,
 * where each label consists of a length octet followed by that
 * number of octets.
 * (https://www.freesoft.org/CIE/RFC/1035/41.htm)
 * 
 * e.g. "google.com" becomes "\6google\3com\0"
 **/
std::string DNS::url_to_qname(std::string& url)
{
  std::string result = "";
  std::string tmp = "";
  int i = 0;
  for (char c : url) {
    if (c == '.') {
      result += static_cast<char>(i) + tmp;
      i = 0;
      tmp = "";
      continue;
    }
    i++;
    tmp += c;
  }
  result += static_cast<char>(i) + tmp + '\0';
  return result;
}

/**
 * Reverse of "url_to_qname"
 **/
std::string DNS::qname_to_url(unsigned char* qname)
{
  std::string url = "";
  for (int i = 1; qname[i] != '\0'; i++) {
    // 48 to 57 -> 0 to 9
    // 65 to 90 -> A to Z
    // 97 to 122 -> a to z
    // 45 -> hyphen
    if (
      (qname[i] >= 48 && qname[i] <= 57) ||
      (qname[i] >= 65 && qname[i] <= 90) ||
      (qname[i] >= 97 && qname[i] <= 122) ||
      qname[i] == 45) {
      url += qname[i];
    } else {
      url += '.';
    }
  }
  return url;
}

int main(int argc, char *argv[])
{
  DNS dns;
  std::string domain = "die.net";
  if (argc > 1) {
    domain = argv[1];
  }
  try {
    dns.get(domain);
  } catch (...) {
    std::cerr << "Something horrible happened.\n";
    return 1;
  }
  return 0;
}