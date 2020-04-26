#pragma once

#ifndef _EASYPX_H_
#define _EASYPX_H_
#include <easyx.h>
#include <map>
#include <string>
#include <memory>
#include <sstream>
#include <iomanip>

namespace thatboy
{
	namespace EasyPX
	{
		// EasyX Window Extent Properties 
		enum :int
		{
			EW_HASMAXIMIZI = 0X10    // Enable the maximize button
			, EW_HASSIZEBOX = 0X20   // Enable the sizebox
			, EW_SWITCHUSKEBOARD = 0X40

			, EW_EXALLPROP = EW_HASMAXIMIZI | EW_HASSIZEBOX | EW_SWITCHUSKEBOARD
		};

		// 
		namespace _DataPackage {
			typedef LRESULT CALLBACK CallBackType(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
			CallBackType* OLD_EasyXProc;
			LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
			{
				switch (msg)
				{
				case WM_SIZE:
					Resize(nullptr, LOWORD(lParam), HIWORD(lParam));
					break;
				default:
					return CallWindowProc(_DataPackage::OLD_EasyXProc, hwnd, msg, wParam, lParam);
				}
				return 0;
			}
		}


		HWND initgraph(int width, int height, int flag = NULL)	// Initialize the graphics environment.
		{
			HWND hEasyX = ::initgraph(width, height, flag & ~EW_EXALLPROP);
			if (flag & EW_SWITCHUSKEBOARD)
				LoadKeyboardLayout(TEXT("0x0409"), KLF_ACTIVATE | KLF_SETFORPROCESS);
			_DataPackage::OLD_EasyXProc = reinterpret_cast<_DataPackage::CallBackType*>(GetWindowLong(hEasyX, GWL_WNDPROC));
			SetWindowLong(hEasyX, GWL_WNDPROC, reinterpret_cast<LONG>(_DataPackage::WndProc));
			SetWindowLong(hEasyX, GWL_STYLE, GetWindowLong(hEasyX, GWL_STYLE) | (flag & EW_HASMAXIMIZI ? WS_MAXIMIZEBOX : 0) | (flag & EW_HASSIZEBOX ? WS_SIZEBOX : 0));
			return hEasyX;
		}



		namespace UI
		{
#ifdef UNICODE
			using string = std::wstring;
			using ostringstream = std::wostringstream;
#else
			using string = std::string;
			using ostringstream = std::ostringstream;
#endif // UNICODE

			class InfoBox
			{
			private:
				struct _info
				{
					_info() = default;
					_info(const string& s) :info(s) {}

					bool show = true;
					string info;
					string arg;
					COLORREF color = BLACK;
					COLORREF argColor = 0X686868;
				};

				void updatePositon()
				{
					float xs, ys;
					getaspectratio(&xs, &ys);
					setaspectratio(1, 1);

					int hasArgNumber = 0;

					maxInfoWidth = 0;
					maxArgWidth = 0;

					textHeight = textheight(TEXT(" ")) + 5;

					for (const auto& info : drawList)
					{
						if (!info->show)
							continue;
						auto thisInfoWidth = textwidth(info->info.c_str());
						auto thisArgWidth = textwidth(info->arg.c_str());

						if (!info->arg.empty())
						{
							++hasArgNumber;
							if (thisInfoWidth > maxInfoWidth)
								maxInfoWidth = thisInfoWidth;
							if (thisArgWidth > maxArgWidth)
								maxArgWidth = thisArgWidth;
						}
					}
					textWidth = maxInfoWidth + 5 + maxArgWidth;

					infoBegin = 5;
					argBegin = infoBegin + maxInfoWidth + 5;

					boxWidth = infoBegin + textWidth + 5;
					boxHeight = 5 + hasArgNumber * textHeight;

					for (const auto& info : drawList)
						if (info->show&&info->arg.empty())
						{
							RECT rt{ infoBegin,0, infoBegin + textWidth,0 };
							boxHeight += drawtext(info->info.c_str(), &rt, DT_LEFT | DT_WORDBREAK | DT_CALCRECT) + 5;
						}
					setaspectratio(xs, ys);
				}
			public:

				template<typename T>
				string fmtString(T in, int width = 0, int prec = 0) {
					ostringstream s;
					s << std::setw(width) << std::setprecision(prec) << in;
					return s.str();
				}

				template<typename T>
				void setInfo(string id, T info, bool resize = false, int width = 0, int prec = 0)
				{
					if (infos.find(id) == infos.end())
						return;
					infos[id]->info = fmtString(info, width, prec);
					if (resize)
						updatePositon();
				}

				template<typename T>
				void setArg(string id, T arg, bool resize = false , int width = 0, int prec = 0)
				{
					if (infos.find(id) == infos.end())
						return;

					infos[id]->arg = fmtString(arg, width, prec);
					if (resize)
						updatePositon();
				}

				template<typename T>
				void addInfo(string id, T info, int width = 0, int prec = 0)
				{
					if (infos.find(id) == infos.end())
					{
						drawList.push_back(std::make_shared<_info>());
						infos[id] = drawList.back();
					}
					setInfo(id, info, width, prec);
					updatePositon();
				}

				void showInfo(string id, bool bShow = true)
				{
					if (infos.find(id) != infos.end())
					{
						auto& ref = *infos[id];
						if (ref.show != bShow)
						{
							ref.show = bShow;
							updatePositon();
						}
					}
				}

				void setInfoColor(string id, COLORREF color = BLACK)
				{
					if (infos.find(id) != infos.end())
						infos[id]->color = color;
				}

				void setArgColor(string id, COLORREF color = BLACK)
				{
					if (infos.find(id) != infos.end())
						infos[id]->argColor = color;
				}

				void drawInfo()const
				{
					drawInfo(x, y);
				}

				void drawInfo(int x, int y)const
				{
					float xs, ys;
					getaspectratio(&xs, &ys);
					setaspectratio(1, 1);

					setlinestyle(PS_SOLID, lineWidth);
					setlinecolor(lineColor);
					setfillcolor(fillColor);
					fillrectangle(x, y, x + boxWidth, y + boxHeight);

					y += 5;

					for (const auto& info : drawList)
					{
						if (!info->show)
							continue;
						else if (info->arg.empty())
						{
							settextcolor(info->color);
							RECT rt{ x + infoBegin,y,x + infoBegin + textWidth,y + (textwidth(info->info.c_str()) / textWidth + 1) * textHeight - 5 };
							drawtext(info->info.c_str(), &rt, DT_LEFT | DT_WORDBREAK);
							y = rt.bottom + 5;
						}
						else
						{
							settextcolor(info->color);
							outtextxy(x + infoBegin, y, info->info.c_str());
							settextcolor(info->argColor);
							outtextxy(x + argBegin, y, info->arg.c_str());
							y += textHeight;
						}
					}
					setaspectratio(xs, ys);
				}

				int getInfoBoxWidth()const
				{
					return boxWidth;
				}

				int getInfoBoxHeight()const
				{
					return boxHeight;
				}

				mutable int x;
				mutable int y;
			protected:
				int lineWidth = 4;
				COLORREF lineColor = 0X252525;
				COLORREF fillColor = 0XE0E0E0;

				int maxInfoWidth = 0;
				int maxArgWidth = 0;
				int textWidth = 0;
				int textHeight = 0;
				int argBegin = 0;
				int infoBegin = 0;
				int boxWidth = 0;
				int boxHeight = 0;
				std::vector<std::shared_ptr<_info>> drawList;
				std::map<string, std::shared_ptr<_info>> infos;
			};
		}
	}

}

#endif