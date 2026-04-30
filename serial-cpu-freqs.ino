/*
   Simple Sketch for testing HardwareSerial with different CPU Frequencies
*/

#include <WiFi.h>
#include <time.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <HTTPUpdate.h>
#include <WiFiClientSecure.h>
#include <cstring>

// 独立于库内默认全局 httpUpdate（默认 ~8s 超时易导致大镜像失败）：OTA 用大超时（毫秒）
static HTTPUpdate otaHttps(180000);

// const char* ssid = "CAT-2.4G";
// const char* password = "qwertyuiop";

const char* ssid = "BzLv15ProMax";
const char* password = "12341234";

const String current_version = "1.0.0";

#define OTA_MANIFEST_URL_PRIMARY \
  "https://cdn.jsdelivr.net/gh/JasonWong08/serial-cpu-freqs@main/version.json"

// GitHub Raw 备选（cdn 偶发抖动时可通）
#define OTA_MANIFEST_URL_FALLBACK \
  "https://raw.githubusercontent.com/JasonWong08/serial-cpu-freqs/main/version.json"

// 固件须放在仓库 firmware/ 并已 push。HTTPUpdate 必须有 Content-Length；jsDelivr 常 chunked 导致 -101。
//  "https://raw.githubusercontent.com/JasonWong08/serial-cpu-freqs/main/firmware/Serial_CPU_Freqs.ino.bin"
#define OTA_FIRMWARE_URL_RAWGH \
  "https://raw.githubusercontent.com/JasonWong08/serial-cpu-freqs/releases/download/v1.1.0/serial-cpu-freqs.ino.bin"  

// "https://cdn.jsdelivr.net/gh/JasonWong08/serial-cpu-freqs@main/firmware/Serial_CPU_Freqs.ino.bin"
#define OTA_FIRMWARE_URL_JSDELIVR \
  "https://cdn.jsdelivr.net/gh/JasonWong08/serial-cpu-freqs/releases/download/v1.1.0/serial-cpu-freqs.ino.bin"

// int cpufreqs = 240;

static void syncNetworkTime() {
  configTime(0, 0, "pool.ntp.org", "time.google.com", "ntp.aliyun.com");
  Serial.print("正在同步 NTP 时间");
  for (int i = 0; i < 60; i++) {
    time_t now = time(nullptr);
    if (now > 1609459200) {
      Serial.println(" 完成");
      Serial.printf("当前 UTC 时间戳: %lld\n", (long long)now);
      return;
    }
    delay(500);
    Serial.print(".");
  }
  Serial.println();
  Serial.println("警告: NTP 未同步成功");
}

void setup() {
  Serial.begin(115200);
  delay(1000);
  Serial.println("\n Starting...\n");
  Serial.flush();

  Serial.printf("\n------- Trying OTA 00 ---------\n");
 
  
  Serial.println("Hello earth!");
  delay(500);

  
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) { delay(500); Serial.print("."); }
  Serial.println("\nWiFi已连接");
  WiFi.setSleep(WIFI_PS_NONE);
  IPAddress ghIp;
  if (WiFi.hostByName("raw.githubusercontent.com", ghIp))
    Serial.printf("DNS raw.githubusercontent.com -> %s\n", ghIp.toString().c_str());

  syncNetworkTime();
}

// Merged PEM: DigiCert G2 / ISRG X1 / GlobalSign R3 / USERTrust RSA (jsDelivr/GitHub POPs)
const char* rootCACertificate = \
"-----BEGIN CERTIFICATE-----\n" \
"MIIDjjCCAnagAwIBAgIQAzrx5qcRqaC7KGSxHQn65TANBgkqhkiG9w0BAQsFADBh\n" \
"MQswCQYDVQQGEwJVUzEVMBMGA1UEChMMRGlnaUNlcnQgSW5jMRkwFwYDVQQLExB3\n" \
"d3cuZGlnaWNlcnQuY29tMSAwHgYDVQQDExdEaWdpQ2VydCBHbG9iYWwgUm9vdCBH\n" \
"MjAeFw0xMzA4MDExMjAwMDBaFw0zODAxMTUxMjAwMDBaMGExCzAJBgNVBAYTAlVT\n" \
"MRUwEwYDVQQKEwxEaWdpQ2VydCBJbmMxGTAXBgNVBAsTEHd3dy5kaWdpY2VydC5j\n" \
"b20xIDAeBgNVBAMTF0RpZ2lDZXJ0IEdsb2JhbCBSb290IEcyMIIBIjANBgkqhkiG\n" \
"9w0BAQEFAAOCAQ8AMIIBCgKCAQEAuzfNNNx7a8myaJCtSnX/RrohCgiN9RlUyfuI\n" \
"2/Ou8jqJkTx65qsGGmvPrC3oXgkkRLpimn7Wo6h+4FR1IAWsULecYxpsMNzaHxmx\n" \
"1x7e/dfgy5SDN67sH0NO3Xss0r0upS/kqbitOtSZpLYl6ZtrAGCSYP9PIUkY92eQ\n" \
"q2EGnI/yuum06ZIya7XzV+hdG82MHauVBJVJ8zUtluNJbd134/tJS7SsVQepj5Wz\n" \
"tCO7TG1F8PapspUwtP1MVYwnSlcUfIKdzXOS0xZKBgyMUNGPHgm+F6HmIcr9g+UQ\n" \
"vIOlCsRnKPZzFBQ9RnbDhxSJITRNrw9FDKZJobq7nMWxM4MphQIDAQABo0IwQDAP\n" \
"BgNVHRMBAf8EBTADAQH/MA4GA1UdDwEB/wQEAwIBhjAdBgNVHQ4EFgQUTiJUIBiV\n" \
"5uNu5g/6+rkS7QYXjzkwDQYJKoZIhvcNAQELBQADggEBAGBnKJRvDkhj6zHd6mcY\n" \
"d5J9PMWLSn/pvtsrF9+wX3N3KjITOYFnQoQj8kVnNeyIv/iPsGEMNKSuIEyExtv4\n" \
"NeF22d+mQrvHRAiGfzZ0JFrabA0UWTW98kndth/Jsw1HKj2ZL7tcu7XUIOGZX1NG\n" \
"Fdtom/DzMNU+MeKNhJ7jitralj41E6Vf8PlwUHBHQRFXGU7Aj64GxJUTFy8bJZ91\n" \
"8rGOmaFvE7FBcf6IKshPECBV1/MUReXgRPTqh5Uykw7+U0b6LJ3/iyK5S9kJRaTe\n" \
"pLiaWN0bfVKfjllDiIGknibVb63dDcY3fe0Dkhvld1927jyNxF1WW6LZZm6zNTfl\n" \
"MrY=\n" \
"-----END CERTIFICATE-----\n" \
"\n" \
"-----BEGIN CERTIFICATE-----\n" \
"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\n" \
"-----END CERTIFICATE-----\n" \
"\n" \
"-----BEGIN CERTIFICATE-----\n" \
"MIIDXzCCAkegAwIBAgILBAAAAAABIVhTCKIwDQYJKoZIhvcNAQELBQAwTDEgMB4GA1UECxMXR2xv\n" \
"YmFsU2lnbiBSb290IENBIC0gUjMxEzARBgNVBAoTCkdsb2JhbFNpZ24xEzARBgNVBAMTCkdsb2Jh\n" \
"bFNpZ24wHhcNMDkwMzE4MTAwMDAwWhcNMjkwMzE4MTAwMDAwWjBMMSAwHgYDVQQLExdHbG9iYWxT\n" \
"aWduIFJvb3QgQ0EgLSBSMzETMBEGA1UEChMKR2xvYmFsU2lnbjETMBEGA1UEAxMKR2xvYmFsU2ln\n" \
"bjCCASIwDQYJKoZIhvcNAQEBBQADggEPADCCAQoCggEBAMwldpB5BngiFvXAg7aEyiie/QV2EcWt\n" \
"iHL8RgJDx7KKnQRfJMsuS+FggkbhUqsMgUdwbN1k0ev1LKMPgj0MK66X17YUhhB5uzsTgHeMCOFJ\n" \
"0mpiLx9e+pZo34knlTifBtc+ycsmWQ1z3rDI6SYOgxXG71uL0gRgykmmKPZpO/bLyCiR5Z2KYVc3\n" \
"rHQU3HTgOu5yLy6c+9C7v/U9AOEGM+iCK65TpjoWc4zdQQ4gOsC0p6Hpsk+QLjJg6VfLuQSSaGjl\n" \
"OCZgdbKfd/+RFO+uIEn8rUAVSNECMWEZXriX7613t2Saer9fwRPvm2L7DWzgVGkWqQPabumDk3F2\n" \
"xmmFghcCAwEAAaNCMEAwDgYDVR0PAQH/BAQDAgEGMA8GA1UdEwEB/wQFMAMBAf8wHQYDVR0OBBYE\n" \
"FI/wS3+oLkUkrk1Q+mOai97i3Ru8MA0GCSqGSIb3DQEBCwUAA4IBAQBLQNvAUKr+yAzv95ZURUm7\n" \
"lgAJQayzE4aGKAczymvmdLm6AC2upArT9fHxD4q/c2dKg8dEe3jgr25sbwMpjjM5RcOO5LlXbKr8\n" \
"EpbsU8Yt5CRsuZRj+9xTaGdWPoO4zzUhw8lo/s7awlOqzJCK6fBdRoyV3XpYKBovHd7NADdBj+1E\n" \
"bddTKJd+82cEHhXXipa0095MJ6RMG3NzdvQXmcIfeg7jLQitChws/zyrVQ4PkX4268NXSb7hLi18\n" \
"YIvDQVETI53O9zJrlAGomecsMx86OyXShkDOOyyGeMlhLxS67ttVb9+E7gUJTb0o2HLO02JQZR7r\n" \
"kpeDMdmztcpHWD9f\n" \
"-----END CERTIFICATE-----\n" \
"\n" \
"-----BEGIN CERTIFICATE-----\n" \
"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\n" \
"Q+6IHdfGjjxDah2nGN59PRbxYvnKkKj9\n" \
"-----END CERTIFICATE-----\n";


void performOTA() {
  if (WiFi.status() != WL_CONNECTED) return;

  WiFiClientSecure client;
  client.setCACert(rootCACertificate);
  client.setTimeout(25);

  HTTPClient http;
  http.setTimeout(25000);
  http.setFollowRedirects(HTTPC_STRICT_FOLLOW_REDIRECTS);

  Serial.println("正在检查更新...");
  const char* manifestUrls[] = { OTA_MANIFEST_URL_PRIMARY, OTA_MANIFEST_URL_FALLBACK };
  int httpCode = -1;
  for (size_t i = 0; i < sizeof(manifestUrls) / sizeof(manifestUrls[0]); i++) {
    if (!http.begin(client, manifestUrls[i])) continue;
    httpCode = http.GET();
    if (httpCode == HTTP_CODE_OK) break;
    http.end();
    client.stop();
    delay(200);
  }

  if (httpCode == HTTP_CODE_OK) {
    String payload = http.getString();
    StaticJsonDocument<512> doc;
    DeserializationError error = deserializeJson(doc, payload);

    if (error) {
      Serial.print("JSON解析失败: ");
      Serial.println(error.c_str());
      http.end();
      return;
    }

    const char* new_version = doc["version"];
    const char* download_url = doc["url"];

    // manifest 若为 Release 直链：改用 GitHub Raw（一般有 Content-Length）；jsDelivr 常 chunked 触发 -101
    String binUrl;
    if (download_url &&
        strstr(download_url, "github.com") &&
        strstr(download_url, "releases/download")) {
      binUrl = OTA_FIRMWARE_URL_RAWGH;
      Serial.println(
          "manifest 中为 GitHub Release 链接，改用 Raw 固件 URL（避免 jsDelivr 无 Content-Length）；");
      Serial.println(
          "请确认 main 已包含 firmware/Serial_CPU_Freqs.ino.bin 并已 push。");
    } else if (download_url && strlen(download_url) > 0) {
      binUrl = download_url;
    } else {
      binUrl = OTA_FIRMWARE_URL_RAWGH;
    }

    if (String(new_version) != current_version) {
      Serial.printf("发现新版本: %s (当前: %s)\n", new_version, current_version.c_str());
      Serial.println("准备开始 OTA 更新...");
      http.end();

      const char* try1 = binUrl.c_str();
      const char* try2 =
          (strcmp(try1, OTA_FIRMWARE_URL_RAWGH) != 0) ? OTA_FIRMWARE_URL_RAWGH : nullptr;
      const char* try3 =
          (strcmp(try1, OTA_FIRMWARE_URL_JSDELIVR) != 0) ? OTA_FIRMWARE_URL_JSDELIVR : nullptr;

      const char* urls[3];
      size_t nu = 0;
      urls[nu++] = try1;
      if (try2) urls[nu++] = try2;
      if (try3 && (!try2 || strcmp(try2, try3) != 0)) urls[nu++] = try3;

      t_httpUpdate_return ret = HTTP_UPDATE_FAILED;
      for (size_t ui = 0; ui < nu; ui++) {
        Serial.printf("实际下载 #%u: %s\n", (unsigned)(ui + 1), urls[ui]);
        WiFiClientSecure otaClient;
        otaClient.setCACert(rootCACertificate);
        otaClient.setTimeout(120);
        otaClient.setHandshakeTimeout(60);
        otaHttps.setFollowRedirects(HTTPC_STRICT_FOLLOW_REDIRECTS);
        otaHttps.setLedPin(2);
        ret = otaHttps.update(otaClient, String(urls[ui]));
        if (ret == HTTP_UPDATE_OK) break;
        int last = otaHttps.getLastError();
        Serial.printf("(失败 %d) %s\n", last, otaHttps.getLastErrorString().c_str());
        if (last != -101) break;
      }

      switch (ret) {
        case HTTP_UPDATE_FAILED:
          Serial.printf("更新失败 (%d): %s\n",
                        otaHttps.getLastError(), otaHttps.getLastErrorString().c_str());
          break;
        case HTTP_UPDATE_NO_UPDATES:
          Serial.println("没有检测到更新");
          break;
        case HTTP_UPDATE_OK:
          Serial.println("更新成功，设备即将重启...");
          break;
      }
    } else {
      Serial.println("设备已是最新版本");
    }
  } else {
    char tlsErr[128] = {0};
    int le = client.lastError(tlsErr, sizeof tlsErr);
    Serial.printf("无法获取版本文件，HTTP: %d (%s)", httpCode,
                  http.errorToString(httpCode).c_str());
    if (le) Serial.printf("，TLS=%d:%s", le, tlsErr);
    Serial.println();
  }
  http.end();
}

void loop() {
  performOTA();
  delay(30000);
}
