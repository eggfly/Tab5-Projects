/*
 Adapted from the Adafruit and Xark's PDQ graphicstest sketch.

 See end of file for original header text and MIT license info.

 This sketch uses the GLCD font only.

 */
#include <esp_heap_caps.h>

#include <M5GFX.h>
#include "WQXUPBMP.h"
#include "WQXUPBMP_1.h"
#include "WQXUPBMP_2.h"
#include "WQXUPBMP_PNG.h"
#include "WQXUPBMP_BMP.h"
#include "wmp6_1280_720.h"

#include <U8g2lib.h>
#include <Arduino_GFX_Library.h>
#include <canvas/Arduino_Canvas.h>

void printMemInfo()
{
  // Internal RAM
  size_t free_internal = heap_caps_get_free_size(MALLOC_CAP_INTERNAL);
  size_t total_internal = heap_caps_get_total_size(MALLOC_CAP_INTERNAL);
  Serial.println("======== Memory Info ========");
  Serial.printf("Internal RAM:\n Free: %u bytes\n Total: %u bytes.\n", free_internal, total_internal);

  // PSRAM
  if (psramFound())
  {
    size_t free_psram = heap_caps_get_free_size(MALLOC_CAP_SPIRAM);
    size_t total_psram = heap_caps_get_total_size(MALLOC_CAP_SPIRAM);
    Serial.printf("PSRAM:\n Free: %u bytes\n Total: %u bytes\n", free_psram, total_psram);
  }
  else
  {
    Serial.println("PSRAM not detected");
  }
}

M5GFX tft;

M5Canvas canvas(&tft);

unsigned long total = 0;
unsigned long tn = 0;

#define SCREEN_WIDTH 720
#define SCREEN_HEIGHT 1280

Arduino_Canvas *gfx = new Arduino_Canvas(SCREEN_WIDTH, SCREEN_HEIGHT, nullptr);

#define PLAYER_SPRITE_WIDTH 640
#define PLAYER_SPRITE_HEIGHT 410
#define PLAYER_SPRITE_TOP_OFFSET 320

Arduino_Canvas *playerSprite = new Arduino_Canvas(PLAYER_SPRITE_WIDTH, PLAYER_SPRITE_HEIGHT, nullptr);

const char three_body_problem[] =
    "罗辑站在叶文洁面前，心中充满了疑惑和不安。他刚刚从叶文洁那里得知了宇宙社会学的核心理论，而这个理论将彻底改变他对宇宙的认知。\n"
    "叶文洁缓缓地说道：“罗辑，你知道宇宙中有多少文明吗？”\n"
    "罗辑摇了摇头：“我不知道，但根据德雷克方程，宇宙中的文明数量应该是非常庞大的。”\n"
    "叶文洁微微一笑：“是的，宇宙中的文明数量确实庞大，但你知道它们为什么没有出现在我们面前吗？”\n"
    "罗辑沉思片刻，回答道：“可能是因为距离太远，或者它们的技术还没有达到能够进行星际旅行的水平。”\n"
    "叶文洁摇了摇头：“不，罗辑，真正的原因远比这更残酷。宇宙中的每一个文明都是猎人，它们隐藏在黑暗中，小心翼翼地隐藏自己，同时也在寻找其他文明。一旦发现其他文明的存在，它们会毫不犹豫地将其消灭。”\n"
    "罗辑震惊地看着叶文洁：“这……这怎么可能？为什么它们要这么做？”\n"
    "叶文洁的目光变得深邃：“因为宇宙的资源是有限的，而文明的发展是无限的。任何一个文明的存在，都会对其他文明构成威胁。为了生存，它们必须消灭潜在的竞争对手。”\n"
    "罗辑感到一阵寒意：“那……那我们该怎么办？如果宇宙真的是这样一个黑暗的森林，我们岂不是随时都可能被其他文明消灭？”\n"
    "叶文洁点了点头：“是的，罗辑，这就是宇宙的真相。我们唯一能做的，就是隐藏自己，不要暴露在宇宙的黑暗中。只有这样，我们才能生存下去。”\n"
    "罗辑沉默了片刻，突然问道：“那……如果我们发现了其他文明，我们该怎么办？”\n"
    "叶文洁的目光变得冰冷：“消灭它，毫不犹豫地消灭它。因为如果你不消灭它，它就会消灭你。”\n"
    "罗辑感到一阵窒息，他从未想过宇宙竟然如此残酷。他低声问道：“这就是宇宙社会学的核心理论吗？”\n"
    "叶文洁点了点头：“是的，罗辑，这就是‘黑暗森林法则’。宇宙是一片黑暗的森林，每一个文明都是带枪的猎人，像幽灵般潜行于林间，轻轻拨开挡路的树枝，竭力不让脚步发出一点儿声音，连呼吸都必须小心翼翼。他必须小心，因为林中到处都有与他一样潜行的猎人。如果他发现了别的生命，能做的只有一件事：开枪消灭之。在这片森林中，他人就是地狱，就是永恒的威胁，任何暴露自己存在的生命都将很快被消灭。”\n"
    "罗辑感到一阵深深的无力感，他意识到自己正站在一个巨大的谜团面前，而这个谜团的答案将决定人类文明的命运。";

const char shen_nong_c_str[] =
    "Shen Nong\n\n"
    "神农一生下来就是\n"
    "个水晶肚子，五脏\n"
    "六腑全都能看得一\n"
    "清二楚。那时侯，\n"
    "人们经常因为乱吃\n"
    "东西而生病，甚至\n"
    "丧命。神农决心尝\n"
    "遍所有的东西，好\n"
    "吃的放在身边左边\n"
    "的袋子里，给人吃\n"
    "；\n"
    "不好吃的就放在身\n"
    "子右边的袋子里，\n"
    "作药用。\n"
    "第一次，神农尝了\n"
    "一片小嫩叶。这叶\n"
    "片一落进肚里，就\n"
    "上上下下地把里面\n"
    "各器官擦洗得清清\n"
    "爽爽，\n"
    "象巡查似的，\n"
    "神农把它叫做\n"
    "“查”，\n"
    "就是后人所称的\n"
    "“茶”。\n"
    "神农将它放进右边\n"
    "袋子里。第二次，\n"
    "神农尝了朵蝴蝶样\n"
    "的淡红小花，甜津\n"
    "津的，香味扑鼻，\n"
    "这是“甘草”。他把\n"
    "它放进了左边袋子\n"
    "里。就这样，神农\n"
    "辛苦地尝遍百草，\n"
    "每次中毒，都靠茶\n"
    "来解救。后来，\n"
    "他左边的袋子里花\n"
    "草根叶有四万七千\n"
    "种，右边有三十九\n"
    "万八千种。\n"
    "但有一天，神农尝\n"
    "到了“断肠草”，这\n"
    "种毒草太厉害了，\n"
    "他还来不及吃茶解\n"
    "毒就死了。\n"
    "他是为了拯救人们\n"
    "而牺牲的，人们称\n"
    "他为“药王菩萨”，\n"
    "人间以这个神话故\n"
    "事永远地纪念他。\n";

void setup()
{
  Serial.begin(115200);
  delay(1500);
  Serial.println("setup() called!");
  Serial.println("");
  printMemInfo();
  // It needs 1843200 bytes.
  // gfx_canvas->begin(); // if there's no output gfx (nullptr), it will only malloc sprite buffer

  // To malloc sprite buffer
  playerSprite->begin();
  playerSprite->fillScreen(MSB_16(TFT_YELLOW));

  printMemInfo();
  Serial.println("M5GFX library Test!");

  // tft.setBrightness(1);
  tft.init();
  Serial.printf("tft.init()\n");

  canvas.setColorDepth(16);
  canvas.createSprite(SCREEN_WIDTH, SCREEN_HEIGHT);
  canvas.createPalette();
  canvas.setPaletteColor(1, TFT_WHITE);
  canvas.setPivot(canvas.width() / 2 - 0.5, canvas.height() / 2 - 0.5);
  // canvas.fillScreen(TFT_DARKGRAY);
  canvas.drawPng(wmp6_1280_720, 40879, 0, 0);
  void *sharedCanvasBuffer = canvas.getBuffer();
  // eggfly modified branch has this function
  gfx->setFramebuffer((uint16_t *)sharedCanvasBuffer);
  playerSprite->setUTF8Print(true); // enable UTF8 support for the Arduino print() function


  playerSprite->setTextSize(1);
  playerSprite->setTextColor(MSB_16(TFT_RED));
  playerSprite->setCursor(0, 16);
  playerSprite->setFont(u8g2_font_unifont_t_chinese);
  // 有些繁体字用GB2312出不来
  playerSprite->println("Arduino 是一個開源嵌入式硬體平台，用來供用戶製作可互動式的嵌入式專案。此外 Arduino 作為一個開源硬體和開源軟件的公司，同時兼有專案和用戶社群。該公司負責設計和製造Arduino電路板及相關附件。這些產品按照GNU寬通用公共許可證（LGPL）或GNU通用公共許可證（GPL）[1]許可的開源硬體和軟件分發的，Arduino 允許任何人製造 Arduino 板和軟件分發。 Arduino 板可以以預裝的形式商業銷售，也可以作為DIY套件購買。");

  // u8g2_font_unifont_t_gb2312 ->16 height? 304973 bytes
  // u8g2_font_unifont_t_chinese -> 979557 bytes
  // u8g2_font_wqy12_t_gb2312b -> 12 height, 118722 bytes
  // playerSprite->setFont(u8g2_font_wqy12_t_gb2312b);
  playerSprite->setFont(u8g2_font_wqy16_t_gb2312);
  playerSprite->println("Arduino 专案始于2003年，作为意大利伊夫雷亚地区伊夫雷亚互动设计研究所的学生专案，目的是为新手和专业人员提供一种低成本且简单的方法，以建立使用感测器与环境相互作用的装置执行器。适用于初学者爱好者的此类装置的常见范例包括感测器、简单机械人、恒温器和运动检测器。");
  playerSprite->setTextColor(MSB_16(TFT_BLACK));
  playerSprite->println(three_body_problem);

  gfx->draw16bitRGBBitmap(
      (SCREEN_WIDTH - PLAYER_SPRITE_WIDTH) / 2, PLAYER_SPRITE_TOP_OFFSET,
      playerSprite->getFramebuffer(), PLAYER_SPRITE_WIDTH, PLAYER_SPRITE_HEIGHT);
  // canvas.fillScreen(TFT_RED);
  // canvas.fillRect(0, 0, 720, 720, TFT_RED);
  // 720 x 894
  // canvas.drawBmp(WQXUPBMP_BMP, 1931096, 0, 0);
  // canvas.drawPng(WQXUPBMP_PNG, 693673, 0, (1280 - 894) / 2);
  // canvas.drawBmp(WQXUPBMP_BMP, 0, 0);
  tft.startWrite();
  // canvas.pushSprite(0, 0);
  // tft.pushImage(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, canvas.getBuffer()); // 这样不行，因为重载不对
  tft.pushImage(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, gfx->getFramebuffer()); // 必须这样 uint16_t*
  // canvas.pushSprite(&tft, 0, 0);
  tft.endWrite();
  delay(30 * 1000);
}

void printnice(int32_t v)
{
  char str[32] = {0};
  sprintf(str, "%d", v);
  for (char *p = (str + strlen(str)) - 3; p > str; p -= 3)
  {
    memmove(p + 1, p, strlen(p) + 1);
    *p = ',';
  }
  while (strlen(str) < 10)
  {
    memmove(str + 1, str, strlen(str) + 1);
    *str = ' ';
  }
  tft.println(str);
}

static inline uint32_t micros_start() __attribute__((always_inline));
static inline uint32_t micros_start()
{
  uint8_t oms = millis();
  while ((uint8_t)millis() == oms)
    ;
  return micros();
}

uint32_t testHaD()
{
  // pseudo-code for cheesy RLE
  // start with color1
  // while more input data remaining
  // 	count =  0nnnnnnn = 1 byte or 1nnnnnnn nnnnnnnn 2 bytes (0 - 32767)
  // 	repeat color count times
  // 	toggle color1/color2
  static const uint8_t HaD_240x320[] PROGMEM =
      {
          0xb9,
          0x50,
          0x0e,
          0x80,
          0x93,
          0x0e,
          0x41,
          0x11,
          0x80,
          0x8d,
          0x11,
          0x42,
          0x12,
          0x80,
          0x89,
          0x12,
          0x45,
          0x12,
          0x80,
          0x85,
          0x12,
          0x48,
          0x12,
          0x80,
          0x83,
          0x12,
          0x4a,
          0x13,
          0x7f,
          0x13,
          0x4c,
          0x13,
          0x7d,
          0x13,
          0x4e,
          0x13,
          0x7b,
          0x13,
          0x50,
          0x13,
          0x79,
          0x13,
          0x52,
          0x13,
          0x77,
          0x13,
          0x54,
          0x13,
          0x75,
          0x13,
          0x57,
          0x11,
          0x75,
          0x11,
          0x5a,
          0x11,
          0x73,
          0x11,
          0x5c,
          0x11,
          0x71,
          0x11,
          0x5e,
          0x10,
          0x71,
          0x10,
          0x60,
          0x10,
          0x6f,
          0x10,
          0x61,
          0x10,
          0x6f,
          0x10,
          0x60,
          0x11,
          0x6f,
          0x11,
          0x5e,
          0x13,
          0x6d,
          0x13,
          0x5c,
          0x14,
          0x6d,
          0x14,
          0x5a,
          0x15,
          0x6d,
          0x15,
          0x58,
          0x17,
          0x6b,
          0x17,
          0x37,
          0x01,
          0x1f,
          0x17,
          0x6b,
          0x17,
          0x1f,
          0x01,
          0x17,
          0x02,
          0x1d,
          0x18,
          0x6b,
          0x18,
          0x1d,
          0x02,
          0x17,
          0x03,
          0x1b,
          0x19,
          0x6b,
          0x19,
          0x1b,
          0x03,
          0x17,
          0x05,
          0x18,
          0x1a,
          0x6b,
          0x1a,
          0x18,
          0x05,
          0x17,
          0x06,
          0x16,
          0x1b,
          0x6b,
          0x1b,
          0x16,
          0x06,
          0x17,
          0x07,
          0x14,
          0x1c,
          0x6b,
          0x1c,
          0x14,
          0x07,
          0x17,
          0x08,
          0x12,
          0x1d,
          0x6b,
          0x1d,
          0x12,
          0x08,
          0x17,
          0x09,
          0x10,
          0x1e,
          0x6b,
          0x1e,
          0x10,
          0x09,
          0x17,
          0x0a,
          0x0e,
          0x1f,
          0x6b,
          0x1f,
          0x0e,
          0x0a,
          0x17,
          0x0b,
          0x0c,
          0x20,
          0x6b,
          0x20,
          0x0c,
          0x0b,
          0x17,
          0x0c,
          0x0b,
          0x21,
          0x69,
          0x21,
          0x0b,
          0x0c,
          0x18,
          0x0d,
          0x08,
          0x23,
          0x67,
          0x23,
          0x08,
          0x0d,
          0x19,
          0x0e,
          0x06,
          0x26,
          0x63,
          0x26,
          0x06,
          0x0e,
          0x19,
          0x0f,
          0x04,
          0x28,
          0x61,
          0x28,
          0x04,
          0x0f,
          0x19,
          0x10,
          0x02,
          0x2a,
          0x5f,
          0x2a,
          0x02,
          0x10,
          0x1a,
          0x3c,
          0x5d,
          0x3c,
          0x1b,
          0x3d,
          0x5b,
          0x3d,
          0x1c,
          0x3d,
          0x59,
          0x3d,
          0x1d,
          0x3e,
          0x57,
          0x3e,
          0x1e,
          0x3e,
          0x55,
          0x3e,
          0x1f,
          0x40,
          0x51,
          0x40,
          0x20,
          0x40,
          0x4f,
          0x40,
          0x22,
          0x40,
          0x22,
          0x09,
          0x22,
          0x40,
          0x24,
          0x40,
          0x1a,
          0x17,
          0x1a,
          0x40,
          0x26,
          0x40,
          0x16,
          0x1d,
          0x16,
          0x40,
          0x28,
          0x40,
          0x12,
          0x23,
          0x12,
          0x40,
          0x2a,
          0x40,
          0x0f,
          0x27,
          0x0f,
          0x40,
          0x2c,
          0x41,
          0x0b,
          0x2b,
          0x0b,
          0x41,
          0x2f,
          0x3f,
          0x09,
          0x2f,
          0x09,
          0x3f,
          0x32,
          0x3d,
          0x08,
          0x33,
          0x08,
          0x3d,
          0x35,
          0x3a,
          0x08,
          0x35,
          0x08,
          0x3a,
          0x3a,
          0x36,
          0x07,
          0x39,
          0x07,
          0x36,
          0x41,
          0x09,
          0x05,
          0x23,
          0x07,
          0x3b,
          0x07,
          0x23,
          0x05,
          0x09,
          0x54,
          0x21,
          0x07,
          0x3d,
          0x07,
          0x21,
          0x64,
          0x1f,
          0x06,
          0x41,
          0x06,
          0x1f,
          0x66,
          0x1d,
          0x06,
          0x43,
          0x06,
          0x1d,
          0x68,
          0x1b,
          0x06,
          0x45,
          0x06,
          0x1b,
          0x6b,
          0x18,
          0x06,
          0x47,
          0x06,
          0x18,
          0x6e,
          0x16,
          0x06,
          0x49,
          0x06,
          0x16,
          0x70,
          0x14,
          0x06,
          0x4b,
          0x06,
          0x14,
          0x72,
          0x13,
          0x06,
          0x4b,
          0x06,
          0x13,
          0x74,
          0x11,
          0x06,
          0x4d,
          0x06,
          0x11,
          0x76,
          0x0f,
          0x06,
          0x4f,
          0x06,
          0x0f,
          0x78,
          0x0e,
          0x05,
          0x51,
          0x05,
          0x0e,
          0x7a,
          0x0c,
          0x06,
          0x51,
          0x06,
          0x0c,
          0x7d,
          0x09,
          0x06,
          0x53,
          0x06,
          0x09,
          0x80,
          0x80,
          0x08,
          0x05,
          0x55,
          0x05,
          0x08,
          0x80,
          0x82,
          0x06,
          0x05,
          0x57,
          0x05,
          0x06,
          0x80,
          0x84,
          0x05,
          0x05,
          0x57,
          0x05,
          0x05,
          0x80,
          0x86,
          0x03,
          0x05,
          0x59,
          0x05,
          0x03,
          0x80,
          0x88,
          0x02,
          0x05,
          0x59,
          0x05,
          0x02,
          0x80,
          0x8f,
          0x5b,
          0x80,
          0x95,
          0x5b,
          0x80,
          0x94,
          0x5d,
          0x80,
          0x93,
          0x5d,
          0x80,
          0x92,
          0x5e,
          0x80,
          0x92,
          0x5f,
          0x80,
          0x91,
          0x5f,
          0x80,
          0x90,
          0x61,
          0x80,
          0x8f,
          0x61,
          0x80,
          0x8f,
          0x61,
          0x80,
          0x8e,
          0x63,
          0x80,
          0x8d,
          0x63,
          0x80,
          0x8d,
          0x63,
          0x80,
          0x8d,
          0x63,
          0x80,
          0x8c,
          0x19,
          0x07,
          0x25,
          0x07,
          0x19,
          0x80,
          0x8b,
          0x16,
          0x0d,
          0x1f,
          0x0d,
          0x16,
          0x80,
          0x8b,
          0x14,
          0x11,
          0x1b,
          0x11,
          0x14,
          0x80,
          0x8b,
          0x13,
          0x13,
          0x19,
          0x13,
          0x13,
          0x80,
          0x8b,
          0x12,
          0x15,
          0x17,
          0x15,
          0x12,
          0x80,
          0x8a,
          0x12,
          0x17,
          0x15,
          0x17,
          0x12,
          0x80,
          0x89,
          0x11,
          0x19,
          0x13,
          0x19,
          0x11,
          0x80,
          0x89,
          0x11,
          0x19,
          0x13,
          0x19,
          0x11,
          0x80,
          0x89,
          0x10,
          0x1b,
          0x11,
          0x1b,
          0x10,
          0x80,
          0x89,
          0x0f,
          0x1c,
          0x11,
          0x1c,
          0x0f,
          0x80,
          0x89,
          0x0f,
          0x1c,
          0x11,
          0x1c,
          0x0f,
          0x80,
          0x89,
          0x0f,
          0x1c,
          0x11,
          0x1c,
          0x0f,
          0x80,
          0x89,
          0x0e,
          0x1d,
          0x11,
          0x1d,
          0x0e,
          0x80,
          0x89,
          0x0e,
          0x1c,
          0x13,
          0x1c,
          0x0e,
          0x80,
          0x89,
          0x0e,
          0x1b,
          0x15,
          0x1b,
          0x0e,
          0x80,
          0x89,
          0x0e,
          0x1b,
          0x15,
          0x1b,
          0x0e,
          0x80,
          0x89,
          0x0e,
          0x1a,
          0x17,
          0x1a,
          0x0e,
          0x80,
          0x89,
          0x0e,
          0x18,
          0x1b,
          0x18,
          0x0e,
          0x80,
          0x89,
          0x0e,
          0x16,
          0x1f,
          0x16,
          0x0e,
          0x80,
          0x89,
          0x0e,
          0x14,
          0x23,
          0x14,
          0x0e,
          0x80,
          0x89,
          0x0f,
          0x11,
          0x27,
          0x11,
          0x0f,
          0x80,
          0x89,
          0x0f,
          0x0e,
          0x2d,
          0x0e,
          0x0f,
          0x80,
          0x89,
          0x0f,
          0x0c,
          0x31,
          0x0c,
          0x0f,
          0x80,
          0x89,
          0x0f,
          0x0b,
          0x33,
          0x0b,
          0x0f,
          0x80,
          0x8a,
          0x0f,
          0x09,
          0x35,
          0x09,
          0x0f,
          0x80,
          0x8b,
          0x10,
          0x08,
          0x35,
          0x08,
          0x10,
          0x80,
          0x8b,
          0x10,
          0x07,
          0x37,
          0x07,
          0x10,
          0x80,
          0x8b,
          0x11,
          0x06,
          0x37,
          0x06,
          0x11,
          0x80,
          0x8b,
          0x12,
          0x05,
          0x37,
          0x05,
          0x12,
          0x80,
          0x8c,
          0x13,
          0x03,
          0x1b,
          0x01,
          0x1b,
          0x03,
          0x13,
          0x80,
          0x8d,
          0x30,
          0x03,
          0x30,
          0x80,
          0x8d,
          0x30,
          0x04,
          0x2f,
          0x80,
          0x8d,
          0x2f,
          0x05,
          0x2f,
          0x80,
          0x8e,
          0x2e,
          0x06,
          0x2d,
          0x80,
          0x8f,
          0x2d,
          0x07,
          0x2d,
          0x80,
          0x8f,
          0x2d,
          0x07,
          0x2d,
          0x80,
          0x90,
          0x2c,
          0x08,
          0x2b,
          0x80,
          0x91,
          0x2b,
          0x09,
          0x2b,
          0x80,
          0x8c,
          0x01,
          0x05,
          0x2a,
          0x09,
          0x2a,
          0x05,
          0x01,
          0x80,
          0x85,
          0x03,
          0x05,
          0x2a,
          0x09,
          0x2a,
          0x05,
          0x03,
          0x80,
          0x82,
          0x04,
          0x05,
          0x2a,
          0x09,
          0x2a,
          0x04,
          0x05,
          0x80,
          0x80,
          0x06,
          0x05,
          0x29,
          0x04,
          0x02,
          0x03,
          0x29,
          0x05,
          0x06,
          0x7e,
          0x07,
          0x05,
          0x29,
          0x03,
          0x03,
          0x03,
          0x29,
          0x05,
          0x07,
          0x7c,
          0x09,
          0x05,
          0x28,
          0x02,
          0x05,
          0x02,
          0x28,
          0x05,
          0x09,
          0x7a,
          0x0a,
          0x05,
          0x28,
          0x02,
          0x05,
          0x02,
          0x28,
          0x05,
          0x0a,
          0x78,
          0x0c,
          0x05,
          0x27,
          0x02,
          0x05,
          0x02,
          0x27,
          0x05,
          0x0c,
          0x76,
          0x0d,
          0x06,
          0x26,
          0x01,
          0x07,
          0x01,
          0x26,
          0x06,
          0x0d,
          0x73,
          0x10,
          0x05,
          0x55,
          0x05,
          0x10,
          0x70,
          0x12,
          0x05,
          0x53,
          0x05,
          0x12,
          0x6e,
          0x13,
          0x06,
          0x51,
          0x06,
          0x13,
          0x6c,
          0x15,
          0x05,
          0x51,
          0x05,
          0x15,
          0x6a,
          0x16,
          0x06,
          0x4f,
          0x06,
          0x16,
          0x68,
          0x18,
          0x06,
          0x4d,
          0x06,
          0x18,
          0x66,
          0x1a,
          0x06,
          0x4b,
          0x06,
          0x1a,
          0x64,
          0x1c,
          0x06,
          0x49,
          0x06,
          0x1c,
          0x55,
          0x07,
          0x05,
          0x1e,
          0x06,
          0x49,
          0x06,
          0x1e,
          0x05,
          0x07,
          0x42,
          0x30,
          0x06,
          0x47,
          0x06,
          0x30,
          0x3a,
          0x34,
          0x06,
          0x45,
          0x06,
          0x34,
          0x35,
          0x37,
          0x06,
          0x43,
          0x06,
          0x37,
          0x32,
          0x39,
          0x07,
          0x3f,
          0x07,
          0x39,
          0x2f,
          0x3c,
          0x07,
          0x3d,
          0x07,
          0x3c,
          0x2c,
          0x3e,
          0x07,
          0x3b,
          0x07,
          0x3e,
          0x2a,
          0x40,
          0x06,
          0x3b,
          0x06,
          0x40,
          0x28,
          0x40,
          0x06,
          0x3c,
          0x07,
          0x40,
          0x26,
          0x3f,
          0x08,
          0x3d,
          0x08,
          0x3f,
          0x24,
          0x3f,
          0x09,
          0x3d,
          0x09,
          0x3f,
          0x22,
          0x3f,
          0x0a,
          0x14,
          0x01,
          0x13,
          0x02,
          0x13,
          0x0a,
          0x3f,
          0x20,
          0x3f,
          0x0b,
          0x14,
          0x01,
          0x13,
          0x02,
          0x13,
          0x0b,
          0x3f,
          0x1f,
          0x3e,
          0x0c,
          0x14,
          0x01,
          0x13,
          0x02,
          0x13,
          0x0c,
          0x3e,
          0x1e,
          0x3e,
          0x0d,
          0x13,
          0x02,
          0x13,
          0x02,
          0x13,
          0x0d,
          0x3e,
          0x1d,
          0x3d,
          0x0e,
          0x13,
          0x02,
          0x13,
          0x02,
          0x13,
          0x0e,
          0x3d,
          0x1c,
          0x3c,
          0x11,
          0x11,
          0x04,
          0x11,
          0x04,
          0x11,
          0x11,
          0x3c,
          0x1b,
          0x10,
          0x01,
          0x2a,
          0x12,
          0x11,
          0x04,
          0x11,
          0x04,
          0x11,
          0x12,
          0x2a,
          0x01,
          0x10,
          0x1a,
          0x0f,
          0x04,
          0x28,
          0x14,
          0x0f,
          0x06,
          0x0f,
          0x06,
          0x0f,
          0x14,
          0x28,
          0x04,
          0x0f,
          0x19,
          0x0e,
          0x06,
          0x26,
          0x16,
          0x0d,
          0x08,
          0x0d,
          0x08,
          0x0d,
          0x16,
          0x26,
          0x06,
          0x0e,
          0x19,
          0x0d,
          0x07,
          0x25,
          0x18,
          0x0b,
          0x0a,
          0x0b,
          0x0a,
          0x0b,
          0x18,
          0x25,
          0x07,
          0x0d,
          0x19,
          0x0c,
          0x09,
          0x23,
          0x1c,
          0x06,
          0x0f,
          0x05,
          0x10,
          0x05,
          0x1c,
          0x23,
          0x09,
          0x0c,
          0x18,
          0x0c,
          0x0b,
          0x21,
          0x69,
          0x21,
          0x0b,
          0x0c,
          0x17,
          0x0b,
          0x0d,
          0x1f,
          0x6b,
          0x1f,
          0x0d,
          0x0b,
          0x17,
          0x0a,
          0x0f,
          0x1e,
          0x6b,
          0x1e,
          0x0f,
          0x0a,
          0x17,
          0x09,
          0x11,
          0x1d,
          0x6b,
          0x1d,
          0x11,
          0x09,
          0x17,
          0x07,
          0x14,
          0x1c,
          0x6b,
          0x1c,
          0x14,
          0x07,
          0x17,
          0x06,
          0x16,
          0x1b,
          0x6b,
          0x1b,
          0x16,
          0x06,
          0x17,
          0x05,
          0x18,
          0x1a,
          0x6b,
          0x1a,
          0x18,
          0x05,
          0x17,
          0x04,
          0x1a,
          0x19,
          0x6b,
          0x19,
          0x1a,
          0x04,
          0x17,
          0x03,
          0x1b,
          0x19,
          0x6b,
          0x19,
          0x1b,
          0x03,
          0x17,
          0x02,
          0x1d,
          0x18,
          0x6b,
          0x18,
          0x1d,
          0x02,
          0x37,
          0x17,
          0x6b,
          0x17,
          0x58,
          0x16,
          0x6b,
          0x16,
          0x5a,
          0x14,
          0x6d,
          0x14,
          0x5c,
          0x13,
          0x6d,
          0x13,
          0x5e,
          0x12,
          0x6d,
          0x12,
          0x60,
          0x10,
          0x6f,
          0x10,
          0x61,
          0x10,
          0x6f,
          0x10,
          0x60,
          0x11,
          0x6f,
          0x11,
          0x5e,
          0x11,
          0x71,
          0x11,
          0x5c,
          0x12,
          0x71,
          0x12,
          0x5a,
          0x12,
          0x73,
          0x12,
          0x58,
          0x12,
          0x75,
          0x12,
          0x56,
          0x13,
          0x75,
          0x13,
          0x54,
          0x13,
          0x77,
          0x13,
          0x51,
          0x14,
          0x79,
          0x14,
          0x4e,
          0x14,
          0x7b,
          0x14,
          0x4c,
          0x14,
          0x7d,
          0x14,
          0x4a,
          0x14,
          0x7f,
          0x14,
          0x48,
          0x13,
          0x80,
          0x83,
          0x13,
          0x46,
          0x13,
          0x80,
          0x85,
          0x13,
          0x44,
          0x12,
          0x80,
          0x89,
          0x12,
          0x42,
          0x11,
          0x80,
          0x8d,
          0x11,
          0x40,
          0x0f,
          0x80,
          0x93,
          0x0f,
          0x45,
          0x04,
          0x80,
          0x9d,
          0x04,
          0xb9,
          0x56,
      };

  tft.fillScreen(TFT_BLACK);

  uint32_t start = micros_start();

  for (int i = 0; i < 0x10; i++)
  {
    tft.setAddrWindow(0, 0, 240, 320);

    uint16_t cnt = 0;
    uint16_t color = tft.color565((i << 4) | i, (i << 4) | i, (i << 4) | i);
    uint16_t curcolor = 0;

    const uint8_t *cmp = &HaD_240x320[0];

    tft.startWrite();
    while (cmp < &HaD_240x320[sizeof(HaD_240x320)])
    {
      cnt = pgm_read_byte(cmp++);
      if (cnt & 0x80)
        cnt = ((cnt & 0x7f) << 8) | pgm_read_byte(cmp++);
      tft.pushBlock(curcolor, cnt); // PDQ_GFX has count
      curcolor ^= color;
    }
    tft.endWrite();
  }

  uint32_t t = micros() - start;

  tft.setTextColor(TFT_YELLOW);
  tft.setTextSize(2);
  tft.setCursor(8, 285);
  tft.print(F("http://hackaday.io/"));
  tft.setCursor(96, 302);
  tft.print(F("Xark"));

  delay(2 * 1000L);

  return t;
}

uint32_t testFillScreen()
{
  unsigned long cost = 0;
  // Shortened this tedious test!
  for (int i = 0; i < 5; i++)
  {
    uint32_t start = micros_start();
    tft.startWrite();
    tft.fillScreen(TFT_BLACK);
    tft.fillScreen(TFT_RED);
    tft.fillScreen(TFT_GREEN);
    tft.fillScreen(TFT_BLUE);
    tft.fillScreen(TFT_WHITE);
    tft.endWrite();
    cost = (micros() - start) / 5;
    delay(1000);
  }

  return cost;
}

uint32_t testText()
{
  tft.fillScreen(TFT_BLACK);
  uint32_t start = micros_start();
  tft.setCursor(0, 0);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.setTextSize(1);
  tft.println(F("Hello World!"));
  tft.setTextSize(2);
  tft.setTextColor(tft.color565(0xff, 0x00, 0x00));
  tft.print(F("RED "));
  tft.setTextColor(tft.color565(0x00, 0xff, 0x00));
  tft.print(F("GREEN "));
  tft.setTextColor(tft.color565(0x00, 0x00, 0xff));
  tft.println(F("BLUE"));
  tft.setTextColor(TFT_YELLOW);
  tft.setTextSize(2);
  tft.println(1234.56);
  tft.setTextColor(TFT_RED);
  tft.setTextSize(3);
  tft.println(0xDEADBEEF, HEX);
  tft.println();
  tft.setTextColor(TFT_GREEN);
  tft.setTextSize(5);
  tft.println(F("Groop"));
  tft.setTextSize(2);
  tft.println(F("I implore thee,"));
  tft.setTextColor(TFT_GREEN);
  tft.setTextSize(1);
  tft.println(F("my foonting turlingdromes."));
  tft.println(F("And hooptiously drangle me"));
  tft.println(F("with crinkly bindlewurdles,"));
  tft.println(F("Or I will rend thee"));
  tft.println(F("in the gobberwarts"));
  tft.println(F("with my blurglecruncheon,"));
  tft.println(F("see if I don't!"));
  tft.println(F(""));
  tft.println(F(""));
  tft.setTextColor(TFT_MAGENTA);
  tft.setTextSize(6);
  tft.println(F("Woot!"));
  uint32_t t = micros() - start;
  delay(1000);
  return t;
}

uint32_t testPixels()
{
  int32_t w = tft.width();
  int32_t h = tft.height();

  uint32_t start = micros_start();
  tft.startWrite();
  for (uint16_t y = 0; y < h; y++)
  {
    for (uint16_t x = 0; x < w; x++)
    {
      tft.drawPixel(x, y, tft.color565(x << 3, y << 3, x * y));
    }
  }
  tft.endWrite();
  return micros() - start;
}

uint32_t testLines(uint16_t color)
{
  uint32_t start, t;
  int32_t x1, y1, x2, y2;
  int32_t w = tft.width();
  int32_t h = tft.height();

  tft.fillScreen(TFT_BLACK);

  x1 = y1 = 0;
  y2 = h - 1;

  start = micros_start();

  for (x2 = 0; x2 < w; x2 += 6)
  {
    tft.drawLine(x1, y1, x2, y2, color);
  }

  x2 = w - 1;

  for (y2 = 0; y2 < h; y2 += 6)
  {
    tft.drawLine(x1, y1, x2, y2, color);
  }

  t = micros() - start; // fillScreen doesn't count against timing

  tft.fillScreen(TFT_BLACK);

  x1 = w - 1;
  y1 = 0;
  y2 = h - 1;

  start = micros_start();

  for (x2 = 0; x2 < w; x2 += 6)
  {
    tft.drawLine(x1, y1, x2, y2, color);
  }

  x2 = 0;
  for (y2 = 0; y2 < h; y2 += 6)
  {
    tft.drawLine(x1, y1, x2, y2, color);
  }

  t += micros() - start;

  tft.fillScreen(TFT_BLACK);

  x1 = 0;
  y1 = h - 1;
  y2 = 0;

  start = micros_start();

  for (x2 = 0; x2 < w; x2 += 6)
  {
    tft.drawLine(x1, y1, x2, y2, color);
  }
  x2 = w - 1;
  for (y2 = 0; y2 < h; y2 += 6)
  {
    tft.drawLine(x1, y1, x2, y2, color);
  }
  t += micros() - start;

  tft.fillScreen(TFT_BLACK);

  x1 = w - 1;
  y1 = h - 1;
  y2 = 0;

  start = micros_start();

  for (x2 = 0; x2 < w; x2 += 6)
  {
    tft.drawLine(x1, y1, x2, y2, color);
  }

  x2 = 0;
  for (y2 = 0; y2 < h; y2 += 6)
  {
    tft.drawLine(x1, y1, x2, y2, color);
  }

  t += micros() - start;

  return t;
}

uint32_t testFastLines(uint16_t color1, uint16_t color2)
{
  uint32_t start;
  int32_t x, y;
  int32_t w = tft.width();
  int32_t h = tft.height();

  tft.fillScreen(TFT_BLACK);

  start = micros_start();

  for (y = 0; y < h; y += 5)
    tft.drawFastHLine(0, y, w, color1);
  for (x = 0; x < w; x += 5)
    tft.drawFastVLine(x, 0, h, color2);

  return micros() - start;
}

uint32_t testRects(uint16_t color)
{
  uint32_t start;
  int32_t n, i, i2;
  int32_t cx = tft.width() / 2;
  int32_t cy = tft.height() / 2;

  tft.fillScreen(TFT_BLACK);
  n = min(tft.width(), tft.height());
  start = micros_start();
  for (i = 2; i < n; i += 6)
  {
    i2 = i / 2;
    tft.drawRect(cx - i2, cy - i2, i, i, color);
  }

  return micros() - start;
}

uint32_t testFilledRects(uint16_t color1, uint16_t color2)
{
  uint32_t start, t = 0;
  int32_t n, i, i2;
  int32_t cx = tft.width() / 2 - 1;
  int32_t cy = tft.height() / 2 - 1;

  tft.fillScreen(TFT_BLACK);
  n = min(tft.width(), tft.height());
  for (i = n; i > 0; i -= 6)
  {
    i2 = i / 2;

    start = micros_start();

    tft.fillRect(cx - i2, cy - i2, i, i, color1);

    t += micros() - start;

    // Outlines are not included in timing results
    tft.drawRect(cx - i2, cy - i2, i, i, color2);
  }

  return t;
}

uint32_t testFilledCircles(uint8_t radius, uint16_t color)
{
  uint32_t start;
  int32_t x, y, w = tft.width(), h = tft.height(), r2 = radius * 2;

  tft.fillScreen(TFT_BLACK);

  start = micros_start();

  for (x = radius; x < w; x += r2)
  {
    for (y = radius; y < h; y += r2)
    {
      tft.fillCircle(x, y, radius, color);
    }
  }

  return micros() - start;
}

uint32_t testCircles(uint8_t radius, uint16_t color)
{
  uint32_t start;
  int32_t x, y, r2 = radius * 2;
  int32_t w = tft.width() + radius;
  int32_t h = tft.height() + radius;

  // Screen is not cleared for this one -- this is
  // intentional and does not affect the reported time.
  start = micros_start();

  for (x = 0; x < w; x += r2)
  {
    for (y = 0; y < h; y += r2)
    {
      tft.drawCircle(x, y, radius, color);
    }
  }

  return micros() - start;
}

uint32_t testTriangles()
{
  uint32_t start;
  int32_t n, i;
  int32_t cx = tft.width() / 2 - 1;
  int32_t cy = tft.height() / 2 - 1;

  tft.fillScreen(TFT_BLACK);
  n = min(cx, cy);

  start = micros_start();

  for (i = 0; i < n; i += 5)
  {
    tft.drawTriangle(
        cx, cy - i,     // peak
        cx - i, cy + i, // bottom left
        cx + i, cy + i, // bottom right
        tft.color565(0, 0, i));
  }

  return micros() - start;
}

uint32_t testFilledTriangles()
{
  uint32_t start, t = 0;
  int32_t i;
  int32_t cx = tft.width() / 2 - 1;
  int32_t cy = tft.height() / 2 - 1;

  tft.fillScreen(TFT_BLACK);

  start = micros_start();

  for (i = min(cx, cy); i > 10; i -= 5)
  {
    start = micros_start();
    tft.fillTriangle(cx, cy - i, cx - i, cy + i, cx + i, cy + i,
                     tft.color565(0, i, i));
    t += micros() - start;
    tft.drawTriangle(cx, cy - i, cx - i, cy + i, cx + i, cy + i,
                     tft.color565(i, i, 0));
  }

  return t;
}

uint32_t testRoundRects()
{
  uint32_t start;
  int32_t w, i, i2;
  int32_t cx = tft.width() / 2 - 1;
  int32_t cy = tft.height() / 2 - 1;

  tft.fillScreen(TFT_BLACK);

  w = min(tft.width(), tft.height());

  start = micros_start();

  for (i = 0; i < w; i += 6)
  {
    i2 = i / 2;
    tft.drawRoundRect(cx - i2, cy - i2, i, i, i / 8, tft.color565(i, 0, 0));
  }

  return micros() - start;
}

uint32_t testFilledRoundRects()
{
  uint32_t start;
  int32_t i, i2;
  int32_t cx = tft.width() / 2 - 1;
  int32_t cy = tft.height() / 2 - 1;

  tft.fillScreen(TFT_BLACK);

  start = micros_start();

  for (i = min(tft.width(), tft.height()); i > 20; i -= 6)
  {
    i2 = i / 2;
    tft.fillRoundRect(cx - i2, cy - i2, i, i, i / 8, tft.color565(0, i, 0));
  }

  return micros() - start;
}

/***************************************************
  Original sketch text:

  This is an example sketch for the Adafruit 2.2" SPI display.
  This library works with the Adafruit 2.2" TFT Breakout w/SD card
  ----> http://www.adafruit.com/products/1480

  Check out the links above for our tutorials and wiring diagrams
  These displays use SPI to communicate, 4 or 5 pins are required to
  interface (RST is optional)
  Adafruit invests time and resources providing this open source code,
  please support Adafruit and open-source hardware by purchasing
  products from Adafruit!

  Written by Limor Fried/Ladyada for Adafruit Industries.
  MIT license, all text above must be included in any redistribution
 ****************************************************/

void loop(void)
{
  ESP_LOGI("main", "LOOP()");
  Serial.println(F("Benchmark                Time (microseconds)"));

  uint32_t usecHaD = testHaD();
  Serial.print(F("HaD pushColor            "));
  Serial.println(usecHaD);
  delay(100);

  uint32_t usecFillScreen = testFillScreen();
  Serial.print(F("Screen fill              "));
  Serial.println(usecFillScreen);
  delay(100);

  uint32_t usecText = testText();
  Serial.print(F("Text                     "));
  Serial.println(usecText);
  delay(100);

  uint32_t usecPixels = testPixels();
  Serial.print(F("Pixels                   "));
  Serial.println(usecPixels);
  delay(100);

  uint32_t usecLines = testLines(TFT_BLUE);
  Serial.print(F("Lines                    "));
  Serial.println(usecLines);
  delay(100);

  uint32_t usecFastLines = testFastLines(TFT_RED, TFT_BLUE);
  Serial.print(F("Horiz/Vert Lines         "));
  Serial.println(usecFastLines);
  delay(100);

  uint32_t usecRects = testRects(TFT_GREEN);
  Serial.print(F("Rectangles (outline)     "));
  Serial.println(usecRects);
  delay(100);

  uint32_t usecFilledRects = testFilledRects(TFT_YELLOW, TFT_MAGENTA);
  Serial.print(F("Rectangles (filled)      "));
  Serial.println(usecFilledRects);
  delay(100);

  uint32_t usecFilledCircles = testFilledCircles(10, TFT_MAGENTA);
  Serial.print(F("Circles (filled)         "));
  Serial.println(usecFilledCircles);
  delay(100);

  uint32_t usecCircles = testCircles(10, TFT_WHITE);
  Serial.print(F("Circles (outline)        "));
  Serial.println(usecCircles);
  delay(100);

  uint32_t usecTriangles = testTriangles();
  Serial.print(F("Triangles (outline)      "));
  Serial.println(usecTriangles);
  delay(100);

  uint32_t usecFilledTrangles = testFilledTriangles();
  Serial.print(F("Triangles (filled)       "));
  Serial.println(usecFilledTrangles);
  delay(100);

  uint32_t usecRoundRects = testRoundRects();
  Serial.print(F("Rounded rects (outline)  "));
  Serial.println(usecRoundRects);
  delay(100);

  uint32_t usedFilledRoundRects = testFilledRoundRects();
  Serial.print(F("Rounded rects (filled)   "));
  Serial.println(usedFilledRoundRects);
  delay(100);

  Serial.println(F("Done!"));

  uint16_t c = 4;
  int8_t d = 1;
  for (int32_t i = 0; i < tft.height(); i++)
  {
    tft.drawFastHLine(0, i, tft.width(), c);
    c += d;
    if (c <= 4 || c >= 11)
      d = -d;
  }

  tft.setCursor(0, 0);
  tft.setTextColor(TFT_MAGENTA);
  tft.setTextSize(2);

  tft.println(F("     M5GFX test"));

  tft.setTextSize(1);
  tft.setTextColor(TFT_WHITE);
  tft.println(F(""));
  tft.setTextSize(1);
  tft.println(F(""));
  tft.setTextColor(tft.color565(0x80, 0x80, 0x80));

  tft.println(F(""));

  tft.setTextColor(TFT_GREEN);
  tft.println(F(" Benchmark               microseconds"));
  tft.println(F(""));
  tft.setTextColor(TFT_YELLOW);

  tft.setTextColor(TFT_CYAN);
  tft.setTextSize(1);
  tft.print(F("HaD pushColor      "));
  tft.setTextColor(TFT_YELLOW);
  tft.setTextSize(2);
  printnice(usecHaD);

  tft.setTextColor(TFT_CYAN);
  tft.setTextSize(1);
  tft.print(F("Screen fill        "));
  tft.setTextColor(TFT_YELLOW);
  tft.setTextSize(2);
  printnice(usecFillScreen);

  tft.setTextColor(TFT_CYAN);
  tft.setTextSize(1);
  tft.print(F("Text               "));
  tft.setTextColor(TFT_YELLOW);
  tft.setTextSize(2);
  printnice(usecText);

  tft.setTextColor(TFT_CYAN);
  tft.setTextSize(1);
  tft.print(F("Pixels             "));
  tft.setTextColor(TFT_YELLOW);
  tft.setTextSize(2);
  printnice(usecPixels);

  tft.setTextColor(TFT_CYAN);
  tft.setTextSize(1);
  tft.print(F("Lines              "));
  tft.setTextColor(TFT_YELLOW);
  tft.setTextSize(2);
  printnice(usecLines);

  tft.setTextColor(TFT_CYAN);
  tft.setTextSize(1);
  tft.print(F("Horiz/Vert Lines   "));
  tft.setTextColor(TFT_YELLOW);
  tft.setTextSize(2);
  printnice(usecFastLines);

  tft.setTextColor(TFT_CYAN);
  tft.setTextSize(1);
  tft.print(F("Rectangles         "));
  tft.setTextColor(TFT_YELLOW);
  tft.setTextSize(2);
  printnice(usecRects);

  tft.setTextColor(TFT_CYAN);
  tft.setTextSize(1);
  tft.print(F("Rectangles-filled  "));
  tft.setTextColor(TFT_YELLOW);
  tft.setTextSize(2);
  printnice(usecFilledRects);

  tft.setTextColor(TFT_CYAN);
  tft.setTextSize(1);
  tft.print(F("Circles            "));
  tft.setTextColor(TFT_YELLOW);
  tft.setTextSize(2);
  printnice(usecCircles);

  tft.setTextColor(TFT_CYAN);
  tft.setTextSize(1);
  tft.print(F("Circles-filled     "));
  tft.setTextColor(TFT_YELLOW);
  tft.setTextSize(2);
  printnice(usecFilledCircles);

  tft.setTextColor(TFT_CYAN);
  tft.setTextSize(1);
  tft.print(F("Triangles          "));
  tft.setTextColor(TFT_YELLOW);
  tft.setTextSize(2);
  printnice(usecTriangles);

  tft.setTextColor(TFT_CYAN);
  tft.setTextSize(1);
  tft.print(F("Triangles-filled   "));
  tft.setTextColor(TFT_YELLOW);
  tft.setTextSize(2);
  printnice(usecFilledTrangles);

  tft.setTextColor(TFT_CYAN);
  tft.setTextSize(1);
  tft.print(F("Rounded rects      "));
  tft.setTextColor(TFT_YELLOW);
  tft.setTextSize(2);
  printnice(usecRoundRects);

  tft.setTextColor(TFT_CYAN);
  tft.setTextSize(1);
  tft.print(F("Rounded rects-fill "));
  tft.setTextColor(TFT_YELLOW);
  tft.setTextSize(2);
  printnice(usedFilledRoundRects);

  tft.setTextSize(1);
  tft.println(F(""));
  tft.setTextColor(TFT_GREEN);
  tft.setTextSize(2);
  tft.print(F("Benchmark Complete!"));
  tft.endWrite();

  // sleep 5 seconds
  delay(5 * 1000L);
}
