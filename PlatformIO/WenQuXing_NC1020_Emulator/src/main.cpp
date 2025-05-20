
#include <esp_heap_caps.h>

#include <M5GFX.h>
#include "WQXUPBMP.h"
#include "WQXUPBMP_1.h"
#include "WQXUPBMP_2.h"
#include "WQXUPBMP_PNG.h"
#include "WQXUPBMP_BMP.h"

#include "nc1020.h"
#include "config.h"
#include "wqx_config.h"
#include "app.h"

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
M5GFX &display = tft;

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

uint8_t lcd_buff[1600];

// 320 * 160 / 8
uint8_t *lcd_buff_expanded;

// 640 * 320 / 8
uint8_t *lcd_buff_expanded_4x;

#define SRC_WIDTH 160
#define SRC_HEIGHT 80
#define DEST_WIDTH 320
#define DEST_HEIGHT 160

void NC1020_Init()
{
  // nc1020_keymap_init();
  auto freq = getCpuFrequencyMhz();
  Serial.printf("cpu freq=%dMHz\n", freq);

  lcd_buff_expanded = (uint8_t *)ps_malloc(320 * 160 / 8);
  lcd_buff_expanded_4x = (uint8_t *)ps_malloc(640 * 320 / 8);

  wqx::Initialize(nullptr);
  wqx::LoadNC1020();
}

void flushTFTDisplay();

void setup()
{
  Serial.begin(115200);
  delay(1500);
  Serial.println("setup() called!");
  sdcard_sdmmc_init();
  NC1020_Init();

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
  // canvas.drawPng(wmp6_1280_720, 40879, 0, 0);
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
  canvas.drawPng(WQXUPBMP_PNG, 693673, 0, (1280 - 894) / 2);
  // canvas.drawBmp(WQXUPBMP_BMP, 0, 0);
  // tft.startWrite();
  // // canvas.pushSprite(0, 0);
  // // tft.pushImage(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, canvas.getBuffer()); // 这样不行，因为重载不对
  // tft.pushImage(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, gfx->getFramebuffer()); // 必须这样 uint16_t*
  // // canvas.pushSprite(&tft, 0, 0);
  // tft.endWrite();
  flushTFTDisplay();
  // delay(5 * 1000);
}

static inline uint32_t micros_start() __attribute__((always_inline));
static inline uint32_t micros_start()
{
  uint8_t oms = millis();
  while ((uint8_t)millis() == oms)
    ;
  return micros();
}

void touch_test_loop();
void nc1020_loop();

void loop()
{
  // ESP_LOGI("main", "LOOP()");
  // touch_test_loop();
  nc1020_loop();
}

// eggfly
void enlargeBuffer(uint8_t *src, uint8_t *dest, size_t src_width, size_t src_height, size_t dest_width, size_t dest_height)
{
  memset(dest, 0, dest_width * dest_height / 8);
  for (int src_y = 0; src_y < src_height; src_y++)
  {
    for (int x = 0; x < src_width; x += 8)
    {
      int src_offset = src_y * src_width / 8 + x / 8;
      uint8_t srcByte = src[src_offset];
      for (int i = 0; i < 8; i++)
      {
        uint8_t pixel = (srcByte >> (7 - i)) & 1;
        int src_x = x + i;

        int dest_x0 = 2 * src_x;
        int dest_y0 = 2 * src_y;
        int dest_x1 = dest_x0 + 1;
        int dest_y1 = dest_y0;
        int dest_x2 = dest_x0;
        int dest_y2 = dest_y0 + 1;
        int dest_x3 = dest_x0 + 1;
        int dest_y3 = dest_y0 + 1;

        int dest_offset_0 = dest_y0 * dest_width / 8 + dest_x0 / 8;
        int dest_offset_1 = dest_y1 * dest_width / 8 + dest_x1 / 8;
        int dest_offset_2 = dest_y2 * dest_width / 8 + dest_x2 / 8;
        int dest_offset_3 = dest_y3 * dest_width / 8 + dest_x3 / 8;

        dest[dest_offset_0] |= (pixel << (7 - dest_x0 % 8));
        dest[dest_offset_1] |= (pixel << (7 - dest_x1 % 8));
        dest[dest_offset_2] |= (pixel << (7 - dest_x2 % 8));
        dest[dest_offset_3] |= (pixel << (7 - dest_x3 % 8));
      }
    }
  }
}

void nc1020_loop()
{
  auto start_time = micros();
  size_t slice = 50; // origin is 20, can be 10, 15
  wqx::RunTimeSlice(slice, false);
  if (LOG_LEVEL <= LOG_LEVEL_VERBOSE)
  {
    Serial.printf("slice=%d,cost=%dus\n", slice, micros() - start_time);
  }
  // }
  wqx::CopyLcdBuffer((uint8_t *)lcd_buff);
  // memset(lcd_buff_ex, 0xf0, sizeof(lcd_buff_ex));
  enlargeBuffer(lcd_buff, lcd_buff_expanded, SRC_WIDTH, SRC_HEIGHT, DEST_WIDTH, DEST_HEIGHT);
  enlargeBuffer(lcd_buff_expanded, lcd_buff_expanded_4x, SRC_WIDTH * 2, SRC_HEIGHT * 2, DEST_WIDTH * 2, DEST_HEIGHT * 2);
  // M5.Lcd.drawBitmap(80, 80, 160, 80, (uint8_t*)lcd_buff_ex);
  // canvas.fillRect(0, (172 - 160) / 2, 320, 160, WQX_COLOR_RGB565_BG);
  // canvas.fillScreen(RGB888_TO_RGB565(0x20, 0x20, 0x20));
  // canvas.fillScreen(RGB565_BLACK);
  // canvas.fillScreen(RGB565_WHITE);
  // canvas.drawBitmap(
  //     0,
  //     (240 - 160) / 2,
  //     lcd_buff_expanded, 320, 160, WQX_COLOR_RGB565_FG, WQX_COLOR_RGB565_BG);
  // canvas.drawBitmap(
  //     (SCREEN_WIDTH - 320) / 2, 280,
  //     lcd_buff_expanded, 320, 160, WQX_COLOR_RGB565_FG, WQX_COLOR_RGB565_BG);
  canvas.drawBitmap(
      (SCREEN_WIDTH - 640) / 2, 260,
      lcd_buff_expanded_4x, 640, 320, WQX_COLOR_RGB565_FG, WQX_COLOR_RGB565_BG);
  canvas.setCursor(2, 2);
  canvas.setTextSize(2);
  canvas.setTextColor(RGB565_RED);
  // canvas.printf("BATTERY %.4fV %.3f%%\n", battery_gauge.voltage, battery_gauge.percent);
  // canvas.setTextColor(RGB565_CYAN);
  canvas.setCursor(2, 20);
  canvas.printf("BATTERY VOLTAGE:\n");
  // canvas.drawBitmap(0, 0, lcd_buff, 160, 80, WQX_COLOR_RGB565_FG, WQX_COLOR_RGB565_BG);
  flushTFTDisplay();
}

void flushTFTDisplay()
{
  tft.startWrite();
  // canvas.pushSprite(0, 0);
  tft.pushImage(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, (uint16_t *)canvas.getBuffer()); // 这样不行，因为重载不对
  // tft.pushImage(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, gfx->getFramebuffer()); // 必须这样 uint16_t*
  // canvas.pushSprite(&tft, 0, 0);
  tft.endWrite();
}

void touch_test_loop()
{
  static bool drawed = false;
  lgfx::touch_point_t tp[5];

  int nums = display.getTouchRaw(tp, 5);
  if (nums)
  {
    for (int i = 0; i < nums; ++i)
    {
      display.setCursor(16, 16 + i * 24);
      display.printf("Raw X:%03d  Y:%03d", tp[i].x, tp[i].y);
    }

    display.convertRawXY(tp, nums);

    for (int i = 0; i < nums; ++i)
    {
      display.setCursor(16, 128 + i * 24);
      display.printf("Convert X:%03d  Y:%03d", tp[i].x, tp[i].y);
    }
    display.display();

    display.setColor(display.isEPD() ? TFT_BLACK : TFT_WHITE);
    for (int i = 0; i < nums; ++i)
    {
      int s = tp[i].size + 3;
      switch (tp[i].id)
      {
      case 0:
        display.fillCircle(tp[i].x, tp[i].y, s);
        break;
      case 1:
        display.drawLine(tp[i].x - s, tp[i].y - s, tp[i].x + s, tp[i].y + s);
        display.drawLine(tp[i].x - s, tp[i].y + s, tp[i].x + s, tp[i].y - s);
        break;
      default:
        display.fillTriangle(tp[i].x - s, tp[i].y + s, tp[i].x + s, tp[i].y + s, tp[i].x, tp[i].y - s);
        break;
      }
      display.display();
    }
    drawed = true;
  }
  else if (drawed)
  {
    drawed = false;
    display.waitDisplay();
    display.clear();
    display.display();
  }
  delayMicroseconds(100);
  // vTaskDelay(1);
}
