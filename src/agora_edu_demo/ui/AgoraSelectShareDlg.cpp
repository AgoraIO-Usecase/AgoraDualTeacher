#include "agoraselectsharedlg.h"
#include <Windows.h>
#include <QPainter>
#include <QBitmap>
#include <QtGui/QScreen>
#include <QtGui/QWindow>
#include <QtWidgets/QScrollBar>
#include <QtWinExtras/QtWin>

#define IMAGE_MAX_PIXEL_WIDTH 192
#define IMAGE_MAX_PIXEL_HEIGHT 108
#define IMAGE_MAX_PIXEL_ALL_HEIGHT 135
#define IMAGE_PADDING 5
#define TEXT_PADDING 10
AgoraSelectShareDlg::AgoraSelectShareDlg(QWidget* parent)
    : QDialog(parent), bCapture(true) {
  setWindowFlags(Qt::FramelessWindowHint);
  ui.setupUi(this);
  ui.scrollAreaWidgetContents->setLayout(
      ui.gridLayout);  // show scrollbar, there is a gridLayoutWidget object
                       // defaultly.
  ui.scrollArea->verticalScrollBar()->setStyleSheet(QLatin1String(
      ""
      "QScrollBar:vertical {border: none;background-color: "
      "rgb(240,240,240);width: 17px;margin: 0px 0 0px 0;}"
      " QScrollBar::handle:vertical { background:  rgba(0, 0, 0, 51); "
      "min-height: 20px;width: 8px;border: 1px solid  rgba(0, 0, 0, "
      "51);border-radius: 4px;margin-left:4px;margin-right:4px;}"
      " QScrollBar::add-line:vertical {background-color: "
      "rgb(240,240,240);height: 4px;}"
      " QScrollBar::sub-line:vertical {background-color: "
      "rgb(240,240,240);height: 4px;}"
      " QScrollBar::up-arrow:vertical, QScrollBar::down-arrow:vertical "
      "{height: 0px;}"
      " QScrollBar::add-page:vertical, QScrollBar::sub-page:vertical "
      "{background: none;}"));
}

AgoraSelectShareDlg::~AgoraSelectShareDlg() {}


void AgoraSelectShareDlg::paintEvent(QPaintEvent* event) {
  QBitmap bmp(this->size());
  bmp.fill();
  QPainter painter(&bmp);
  painter.setPen(Qt::black);
  painter.setBrush(Qt::black);
  painter.setRenderHints(QPainter::HighQualityAntialiasing |
                         QPainter::SmoothPixmapTransform);

  QPainterPath path;
  path.setFillRule(Qt::WindingFill);
  path.addRoundedRect(rect(), 8, 8);
  painter.fillPath(path, QBrush(QColor(93, 201, 87)));
  setMask(bmp);

}

/**
 * https://chromium.googlesource.com/external/webrtc/+/lkgr/modules/desktop_capture/window_capturer_win.cc
 */
BOOL CALLBACK EnumWindowsProc(_In_ HWND hwnd, _In_ LPARAM lParam) {
  // Skip windows that are invisible, minimized, have no title, or are owned,
  // unless they have the app window style set.
  HWND owner = GetWindow(hwnd, GW_OWNER);
  LONG exstyle = GetWindowLong(hwnd, GWL_EXSTYLE);
  if (IsIconic(hwnd) || !IsWindowVisible(hwnd) ||
      (owner && !(exstyle & WS_EX_APPWINDOW))) {
    return TRUE;
  }

  // Skip the Program Manager window and the Start button.
  const size_t kClassLength = 256;
  char class_name[kClassLength];
  const int class_name_length = GetClassNameA(hwnd, class_name, kClassLength);

  //    RTC_DCHECK(class_name_length)
  //        << "Error retrieving the application's class name";
  // Skip Program Manager window and the Start button. This is the same logic
  // that's used in Win32WindowPicker in libjingle. Consider filtering other
  // windows as well (e.g. toolbars).
  if (strcmp(class_name, "Progman") == 0 || strcmp(class_name, "Button") == 0)
    return TRUE;

  // Windows 8 introduced a "Modern App" identified by their class name being
  // either ApplicationFrameWindow or windows.UI.Core.coreWindow. The
  // associated windows cannot be captured, so we skip them.
  // http://crbug.com/526883.

  ULONGLONG dwConditionMask = 0;
  VER_SET_CONDITION(dwConditionMask, VER_MAJORVERSION, VER_GREATER_EQUAL);
  VER_SET_CONDITION(dwConditionMask, VER_MINORVERSION, VER_GREATER_EQUAL);
  OSVERSIONINFOEX osx;
  ZeroMemory(&osx, sizeof(OSVERSIONINFOEX));
  osx.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);
  osx.dwMajorVersion = 6;
  osx.dwMinorVersion = 2;
  int ret = VerifyVersionInfo(&osx, VER_MAJORVERSION | VER_MINORVERSION,
                              dwConditionMask);
  // win8
  if (ret && (strcmp(class_name, "ApplicationFrameWindow") == 0 ||
              strcmp(class_name, "Windows.UI.Core.CoreWindow") == 0) ||

      strcmp(class_name, "Chrome_WidgetWin_1") == 0) {
    return TRUE;
  }

  QSet<HWND>* pSet = reinterpret_cast<QSet<HWND>*>(lParam);
  LONG lStyle = ::GetWindowLong(hwnd, GWL_STYLE);
  if ((lStyle & WS_VISIBLE) != 0 && (lStyle & (WS_POPUP | WS_SYSMENU)) != 0) {
    pSet->insert(hwnd);
  }

  return TRUE;
}

bool captureBmpToJpeg(const HWND& hWnd, char* szName,
                      std::vector<CaptureInfo>& wndsInfo) {
#if 0
	WCHAR szFilePath[MAX_PATH] = { 0 };
	wsprintfW(szFilePath, L"D:\\bmp\\%d.jpg", i++);

	if (!szFilePath || !wcslen(szFilePath))
		return false;
#endif
  // calculate the number of color indexes in the color table

  int nBitCount = 32;
  int nColorTableEntries = 0;  // nBitCunt 16 24 32
  HDC hDC = GetDC(hWnd);
  HDC hMemDC = CreateCompatibleDC(hDC);

  int nWidth = 0;
  int nHeight = 0;

  if (hWnd != HWND_DESKTOP) {
    RECT rect;
    ::GetClientRect(hWnd, &rect);
    nWidth = rect.right - rect.left;
    nHeight = rect.bottom - rect.top;
  } else {
    nWidth = ::GetSystemMetrics(SM_CXSCREEN);
    nHeight = ::GetSystemMetrics(SM_CYSCREEN);
  }

  if (nWidth == 0 || nHeight == 0) return false;

  int bmpWidth = IMAGE_MAX_PIXEL_WIDTH;
  int bmpHeight = IMAGE_MAX_PIXEL_HEIGHT;
  /*if (nWidth <= IMAGE_MAX_PIXEL_WIDTH && nHeight < IMAGE_MAX_PIXEL_HEIGHT) {
          bmpWidth = nWidth;
          bmpHeight = nHeight;
  }
  else if (nWidth > nHeight && nWidth > IMAGE_MAX_PIXEL_WIDTH) {
          float rate = nWidth / (float)IMAGE_MAX_PIXEL_WIDTH;
          float h = (float)nHeight / rate;
          bmpWidth = IMAGE_MAX_PIXEL_WIDTH;
          bmpHeight = (int)h;
  }
  else if (nHeight > nWidth && nHeight > IMAGE_MAX_PIXEL_HEIGHT) {
          float rate = nHeight / (float)IMAGE_MAX_PIXEL_HEIGHT;
          float w = (float)nWidth / rate;
          bmpHeight = IMAGE_MAX_PIXEL_HEIGHT;
          bmpWidth = (int)w;
  }*/

  if (fabs((float)nWidth / (float)nHeight - (float)16 / (float)9) < 0.01f) {
    float rate = nWidth / (float)IMAGE_MAX_PIXEL_WIDTH;
    float h = (float)nHeight / rate;
    bmpWidth = IMAGE_MAX_PIXEL_WIDTH;
    bmpHeight = (int)h;
  } else {
    float rate = nHeight / (float)IMAGE_MAX_PIXEL_HEIGHT;
    float w = (float)nWidth / rate;
    bmpHeight = IMAGE_MAX_PIXEL_HEIGHT;
    bmpWidth = (int)w;
  }

  HBITMAP hBMP = CreateCompatibleBitmap(hDC, nWidth, nHeight);
  SelectObject(hMemDC, hBMP);
  SetStretchBltMode(hMemDC, COLORONCOLOR);
  StretchBlt(hMemDC, 0, 0, bmpWidth, bmpHeight, hDC, 0, 0, nWidth, nHeight,
             SRCCOPY);
  int nStructLength =
      sizeof(BITMAPINFOHEADER) + sizeof(RGBQUAD) * nColorTableEntries;
  LPBITMAPINFOHEADER lpBitmapInfoHeader =
      (LPBITMAPINFOHEADER) new char[nStructLength];
  ::ZeroMemory(lpBitmapInfoHeader, nStructLength);

  lpBitmapInfoHeader->biSize = sizeof(BITMAPINFOHEADER);
  lpBitmapInfoHeader->biWidth = bmpWidth;
  lpBitmapInfoHeader->biHeight = bmpHeight;
  lpBitmapInfoHeader->biPlanes = 1;
  lpBitmapInfoHeader->biBitCount = nBitCount;
  lpBitmapInfoHeader->biCompression = BI_RGB;
  lpBitmapInfoHeader->biXPelsPerMeter = 0;
  lpBitmapInfoHeader->biYPelsPerMeter = 0;
  lpBitmapInfoHeader->biClrUsed = nColorTableEntries;
  lpBitmapInfoHeader->biClrImportant = nColorTableEntries;

  DWORD dwBytes = ((DWORD)bmpWidth * nBitCount) / 32;
  if (((DWORD)bmpWidth * nBitCount) % 32) {
    dwBytes++;
  }
  dwBytes *= 4;

  DWORD dwSizeImage = dwBytes * bmpHeight;
  lpBitmapInfoHeader->biSizeImage = dwSizeImage;

  LPBYTE lpDibBits = 0;
  HBITMAP hBitmap =
      ::CreateDIBSection(hMemDC, (LPBITMAPINFO)lpBitmapInfoHeader,
                         DIB_RGB_COLORS, (void**)&lpDibBits, NULL, 0);
  SelectObject(hMemDC, hBitmap);
  SetStretchBltMode(hMemDC, COLORONCOLOR);
  StretchBlt(hMemDC, 0, 0, bmpWidth, bmpHeight, hDC, 0, 0, nWidth, nHeight,
             SRCCOPY);

  ReleaseDC(hWnd, hDC);

  QPixmap pixmap = QtWin::fromHBITMAP(hBitmap);

  CaptureInfo info;
  info.pixmap = pixmap;
  info.title = QString::fromUtf8(szName);  //(szName).toLocal8Bit().data();
  info.width = bmpWidth;
  info.height = bmpHeight;
  info.hwnd = hWnd;
  info.isDesktop = false;
  wndsInfo.push_back(info);

  if (lpBitmapInfoHeader) {
    delete[] lpBitmapInfoHeader;
    lpBitmapInfoHeader = NULL;
  }

  if (hBMP) {
    DeleteObject(hBMP);
    hBMP = NULL;
  }

  if (hBitmap) {
    DeleteObject(hBitmap);
    hBitmap = NULL;
  }

  if (hMemDC) {
    DeleteDC(hMemDC);
    hMemDC = NULL;
  }
  return true;
}

void AgoraSelectShareDlg::getAvailableScreenId() {
  QSet<HWND> winIds;
  // std::unordered_set<HWND> setHwnds;
  winIds.clear();
  EnumWindows(EnumWindowsProc, (LPARAM)(&winIds));

  // int i = 0;
  wndsInfo.clear();
  auto screens = QGuiApplication::screens();
  QRect rc = screens[0]->availableVirtualGeometry();
  QPixmap fullPixmap =
      screens[0]->grabWindow(0, rc.x(), rc.y(), rc.width(), rc.height());

  //
#if 1
  CaptureInfo info;
  QString text = QString::fromLocal8Bit("×ÀÃæ");
  info.title = text;  // QString("%1%2").arg(text).arg(i +
                      // 1);//(szName).toLocal8Bit().data();
  info.width = IMAGE_MAX_PIXEL_WIDTH;
  info.height = IMAGE_MAX_PIXEL_HEIGHT;
  info.isDesktop = true;

  QRect rcScreen = screens[0]->geometry();
  QPixmap pixmap(rcScreen.width(), rcScreen.height());
  QPainter painter(&pixmap);
  pixmap.fill(Qt::white);
  QRect rcDest(0, 0, rcScreen.width(), rcScreen.height());
  painter.drawPixmap(rcDest, fullPixmap, rcScreen);

  info.pixmap = pixmap.scaled(IMAGE_MAX_PIXEL_WIDTH, IMAGE_MAX_PIXEL_HEIGHT);
  qreal dpi = screens[0]->logicalDotsPerInch();
  int w = (int)rcScreen.width() * (dpi / 96.0f);
  int h = rcScreen.height() * (dpi / 96.0f);
  info.rcDesktop = QRect(rcScreen.x(), rcScreen.y(), w, h);

  wndsInfo.push_back(info);
#else  // dual screen
  screens = QGuiApplication::screens();
  rc = screens[0]->availableVirtualGeometry();
  fullPixmap =
      screens[0]->grabWindow(0, rc.x(), rc.y(), rc.width(), rc.height());
  for (int i = 0; i < screens.size(); ++i) {
    CaptureInfo info;
    QString text = QString::fromLocal8Bit("×ÀÃæ");
    info.title = text;  // QString("%1%2").arg(text).arg(i +
                        // 1);//(szName).toLocal8Bit().data();
    info.width = IMAGE_MAX_PIXEL_WIDTH;
    info.height = IMAGE_MAX_PIXEL_HEIGHT;
    info.isDesktop = true;

    QRect rcScreen = screens[i]->geometry();
    QPixmap pixmap(rcScreen.width(), rcScreen.height());
    QPainter painter(&pixmap);
    pixmap.fill(Qt::white);
    QRect rcDest(0, 0, rcScreen.width(), rcScreen.height());
    painter.drawPixmap(rcDest, fullPixmap, rcScreen);

    info.pixmap = pixmap.scaled(IMAGE_MAX_PIXEL_WIDTH, IMAGE_MAX_PIXEL_HEIGHT);
    qreal dpi = screens[i]->logicalDotsPerInch();
    int w = (int)rcScreen.width() * (dpi / 96.0f);
    int h = rcScreen.height() * (dpi / 96.0f);
    info.rcDesktop = QRect(rcScreen.x(), rcScreen.y(), w, h);

    wndsInfo.push_back(info);
  }
#endif

  shareButtons.clear();
  for (auto iter : winIds) {
    char class_name[100] = {0};
    WCHAR szName[MAX_PATH] = {0};
    char name[MAX_PATH] = {0};
    GetWindowTextW(iter, szName, MAX_PATH);
    if (wcslen(szName) == 0)  //
      continue;
    ::WideCharToMultiByte(CP_UTF8, 0, szName, wcslen(szName), name, MAX_PATH,
                          NULL, NULL);
    GetClassNameA(iter, class_name, 99);
    HWND windowid = iter;
    if (strcmp(class_name, "EdgeUiInputTopWndClass") == 0 ||
        strcmp(class_name, "Shell_TrayWnd") == 0 ||
        strcmp(class_name, "DummyDWMListenerWindow") == 0 ||
        strcmp(class_name, "WorkerW") == 0 ||
        strcmp(class_name, "PopupRbWebDialog") == 0 ||
        strcmp(class_name, "TXGuiFoundation") == 0)  // kuwo advertisement
    {
      continue;
    }
    captureBmpToJpeg(windowid, name, wndsInfo);
  }

  int currentIndex = 0;

  for (auto iter : wndsInfo) {
    ShareItemButton* toolButton = new ShareItemButton(ui.gridLayoutWidget);
    // toolButton->setObjectName(QStringLiteral("toolButton"));
    toolButton->setMinimumSize(
        QSize(IMAGE_MAX_PIXEL_WIDTH, IMAGE_MAX_PIXEL_ALL_HEIGHT));
    toolButton->setMaximumSize(
        QSize(IMAGE_MAX_PIXEL_WIDTH, IMAGE_MAX_PIXEL_ALL_HEIGHT));
    toolButton->info = iter;
    toolButton->setFlat(true);
    toolButton->setCheckable(true);
    int row = currentIndex / 3;
    int col = currentIndex % 3;

    ui.gridLayout->addWidget(toolButton, row, col, 1, 1);

    connect(toolButton, &QPushButton::toggled, toolButton,
            &ShareItemButton::sharetoggled);
    connect(toolButton, &ShareItemButton::notifyChecked, this,
            &AgoraSelectShareDlg::onNotify);
    currentIndex++;
    shareButtons.push_back(toolButton);
  }

  return;
}

void AgoraSelectShareDlg::onNotify(bool checked,
                                   ShareItemButton* pSelectButton) {
  if (!checked) {
    if (pSelectButton == selectedButton) bCapture = false;
    return;
  }

  int count = ui.gridLayout->count();
  for (int i = 0; i < shareButtons.size(); i++) {
    ShareItemButton* pButton = shareButtons[i];
    if (pButton->isChecked() && pButton != pSelectButton) {
      pButton->setChecked(false);
    }
  }

  selectedButton = pSelectButton;
  bCapture = true;
}

void AgoraSelectShareDlg::on_applyButton_clicked() {
  for (int i = 0; i < shareButtons.size(); i++) {
    ShareItemButton* pButton = shareButtons[i];
    if (pButton->isChecked()) {
      selectedInfo = pButton->info;
      break;
    }
  }
  accept();
}

void AgoraSelectShareDlg::on_ExitPushButton_clicked() { close(); }

ShareItemButton::ShareItemButton(QWidget* parent)
    : font(QFont("Î¢ÈíÑÅºÚ", 9)), fontMetrics(QFontMetrics(font)) {
  setWindowFlags(Qt::FramelessWindowHint);
}

ShareItemButton::~ShareItemButton() {}

void ShareItemButton::sharetoggled(bool checked) {
  emit notifyChecked(checked, this);
}

void ShareItemButton::paintEvent(QPaintEvent* pe) {
  QPainter* painter = new QPainter(this);
  //	painter->setOpacity(1);

  QRect rc = rect();
  QRect rcBorder(rc.x(), rc.y(), rc.width(), rc.height());
  int offset = (rc.width() - info.width) / 2;
  QRect rcImage(rc.x() + offset, rc.y(), info.width, info.height);
  /*	if (info.isDesktop){
                  QRect rcSource = info.rcDesktop;
                  painter->drawPixmap(rcImage, info.pixmap, rcSource);
          }
          else*/
  painter->drawPixmap(rcImage, info.pixmap);

  QRect rcText(
      rc.x() + TEXT_PADDING, IMAGE_MAX_PIXEL_HEIGHT + IMAGE_PADDING,
      rc.width() - 2 * TEXT_PADDING,
      IMAGE_MAX_PIXEL_ALL_HEIGHT - IMAGE_MAX_PIXEL_HEIGHT - IMAGE_PADDING);
  QRect rcTextBack(rc.x() + 1, IMAGE_MAX_PIXEL_HEIGHT, rc.width() - 2,
                   IMAGE_MAX_PIXEL_ALL_HEIGHT - IMAGE_MAX_PIXEL_HEIGHT - 1);

  QColor brushColor, textColor, borderColor;
  painter->setRenderHint(QPainter::Antialiasing);
  if (isChecked()) {
    brushColor = QColor(0, 155, 255);
    textColor = Qt::white;
    borderColor = QColor(68, 162, 252);
  } else {
    brushColor = Qt::white;
    // text color
    textColor = QColor(51, 51, 51);
    // border color
    borderColor = QColor(204, 204, 204);
  }
  QString text =
      fontMetrics.elidedText(info.title, Qt::ElideRight, rcText.width());
  // back
  QBrush brush(brushColor);
  QPainterPath textBackpath;
  textBackpath.addRoundedRect(rcTextBack, 2, 2);
  painter->fillPath(textBackpath, brush);
  // text
  painter->setPen(textColor);
  painter->drawText(rcText, Qt::AlignCenter, text);
  // border
  painter->setPen(borderColor);
  QPainterPath path;
  path.addRoundedRect(rcBorder, 4, 4);
  painter->drawPath(path);
}
