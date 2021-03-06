#include "stdafx.h"
#include "c_download_dlg.h"
#include "resource.h"
#include <thread>
#include "c_graphic.h"



void init_keyinput()
{}
void deinit_keyinput()
{}

c_download_dlg::c_download_dlg(const std::string& link, wnd_hinstance hIn, wnd_hwnd hParent) :
	m_link(link)
{
	init(hIn, IDD_DIALOG_DOWNLOAD, hParent);

	m_data_size = 0;
	m_downloaded_size = -1;


	m_result = 1;
}

c_download_dlg::~c_download_dlg()
{

}

void download_thread(data_response_t& dt_response, const std::string link)
{
	sp_updater::send_request(link.c_str(), dt_response);
}

wnd_bool c_download_dlg::oninit_dialog()
{
	m_down_data.res_hwnd = m_hwnd;

	m_progress_bar = GetDlgItem(m_hwnd, IDC_PROGRESS_DOWNLOAD);

	std::thread(download_thread, std::ref(m_down_data), m_link).detach();

	return 1;
}

void c_download_dlg::close()
{
	DETROY_WINDOW(m_hwnd);
}

LRESULT c_download_dlg::on_command(wnd_wparam wpram, wnd_lparam lpram)
{
	WORD cmd = GET_WM_COMMAND_ID(wpram, lpram);
	switch (cmd)
	{
	case IDCANCEL:
	{
		if (MessageBox(m_hwnd, L"Bạn có muốn dừng ?", L"EVkey - Auto update", MB_YESNO | MB_ICONINFORMATION) == IDYES)
		{
			m_result = 0;
			close();
		}

		return TRUE;
	}
	break;
	}
	return c_my_dlg::on_command(wpram, lpram);
}

bool rect_screen_to_client(wnd_hwnd hwnd, wnd_rect* rect)
{
	return ::ScreenToClient(hwnd, (wnd_point*)&rect->left) &&
		::ScreenToClient(hwnd, (wnd_point*)&rect->right);
}

bool rect_client_to_screen(wnd_hwnd hwnd, wnd_rect* rect)
{
	return ::ClientToScreen(hwnd, (wnd_point*)&rect->left) &&
		::ClientToScreen(hwnd, (wnd_point*)&rect->right);
}

LRESULT c_download_dlg::dialog_proc(wnd_hwnd hwnd, wnd_uint mess_id, wnd_wparam wpram, wnd_lparam lpram)
{
	switch (mess_id)
	{
	case MSG_PROGRESS_DOWNLOAD:
	{
		int ret = (int)wpram;
		if (ret == -1)
		{
			m_data_size = 0;
			m_downloaded_size = -1;

			close();
		}
		else if (wpram == lpram)
		{
			m_data_size = m_downloaded_size = (int)m_down_data.buffer_cache.size();

			close();
		}
		else
		{
			m_data_size = (int)wpram;
			m_downloaded_size = (int)lpram;

			InvalidateRect(hwnd, nullptr, FALSE);
		}

		return 1;
	}
	break;

	case WM_PAINT:
	{
		HDC main_hdc = GetDC(hwnd);
		HWND progressbar = GetDlgItem(hwnd, IDC_PROGRESS_DOWNLOAD);
		ShowWindow(progressbar, SW_HIDE);

		RECT rect_bar;
		GetWindowRect(progressbar, &rect_bar);
		rect_screen_to_client(hwnd, &rect_bar);

		RECT rect_cli = { 0, 0, fl_rect_width(rect_bar), fl_rect_height(rect_bar) };
		c_dc dcbuff;
		dcbuff.create_compatible(main_hdc, fl_rect_width(rect_bar), fl_rect_height(rect_bar));

		c_brush brush_bkg;
		brush_bkg.m_hbrush = CreateSolidBrush(GetSysColor(COLOR_3DFACE));
		FillRect(dcbuff.m_hdc, &rect_cli, brush_bkg.m_hbrush);

		if (m_data_size > 0 && m_downloaded_size > 0)
		{
			c_brush brush_fill;
			brush_fill.m_hbrush = CreateSolidBrush(RGB(128, 200, 164));
			SelectObject(dcbuff.m_hdc, brush_fill.m_hbrush);
			int w = m_downloaded_size * fl_rect_width(rect_cli) / m_data_size;
			RECT rect_fill = { 0, 0, w, fl_rect_height(rect_cli) };
			FillRect(dcbuff.m_hdc, &rect_fill, brush_fill.m_hbrush);
		}

		c_pen pen;
		pen.m_hpen = CreatePen(PS_SOLID, 1, RGB(64, 64, 64));
		SelectObject(dcbuff.m_hdc, pen.m_hpen);
		SelectObject(dcbuff.m_hdc, GetStockObject(HOLLOW_BRUSH));
		Rectangle(dcbuff.m_hdc, 0, 0, fl_rect_width(rect_bar), fl_rect_height(rect_bar));

		BitBlt(main_hdc, rect_bar.left, rect_bar.top, fl_rect_width(rect_bar), fl_rect_height(rect_bar),
			dcbuff.m_hdc, 0, 0, SRCCOPY);
	}
	}

	return c_my_dlg::dialog_proc(hwnd, mess_id, wpram, lpram);
}
