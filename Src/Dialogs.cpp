// Dialogs.h
//
// Viewer dialogs including cheatsheet, about, save-as, and the image information overlay.
//
// Copyright (c) 2019, 2020 Tristan Grimmer.
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby
// granted, provided that the above copyright notice and this permission notice appear in all copies.
//
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT,
// INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN
// AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
// PERFORMANCE OF THIS SOFTWARE.

#include <GL/glew.h>
#include <GLFW/glfw3.h>				// Include glfw3.h after our OpenGL definitions.
#include <Math/tVector2.h>
#include <Foundation/tVersion.h>
#include <System/tFile.h>
#include "imgui.h"
#include "Dialogs.h"
#include "TacitImage.h"
#include "TacitTexView.h"
using namespace tStd;
using namespace tSystem;
using namespace tMath;


namespace TexView
{
	void SaveImageTo(const tString& outFile, int finalWidth, int finalHeight);
	bool DoOverwriteFileModal(const tString& outFile, int finalWidth, int finalHeight);
}


void TexView::ShowInfoOverlay(bool* popen, float x, float y, float w, float h, int cursorX, int cursorY, float zoom)
{
	// This overlay function is pretty much taken from the DearImGui demo code.
	const float margin = 6.0f;

	tVector2 windowPos = tVector2
	(
		x + ((Config.OverlayCorner & 1) ? w - margin : margin),
		y + ((Config.OverlayCorner & 2) ? h - margin : margin)
	);
	tVector2 windowPivot = tVector2
	(
		(Config.OverlayCorner & 1) ? 1.0f : 0.0f,
		(Config.OverlayCorner & 2) ? 1.0f : 0.0f
	);
	ImGui::SetNextWindowPos(windowPos, ImGuiCond_Always, windowPivot);
	ImGui::SetNextWindowBgAlpha(0.6f);
	ImGuiWindowFlags flags =
		ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize |
		ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings |
		ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNav | ImGuiWindowFlags_NoScrollbar;

	if (ImGui::Begin("InfoOverlay", popen, flags))
	{
		ImGui::Text("Information Overlay");
		ImGui::Text("Right-Click to Change Anchor");
		ImGui::Separator();

		if (CurrImage)
		{
			tColourf floatCol(PixelColour);
			tVector4 colV4(floatCol.R, floatCol.G, floatCol.B, floatCol.A);
			ImGui::Text("Colour"); ImGui::SameLine();
			if (ImGui::ColorButton("Colour##2f", colV4, ImGuiColorEditFlags_RGB | ImGuiColorEditFlags_NoPicker | ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoLabel, tVector2(15,15)))
				ImGui::OpenPopup("CopyColourOverlayAs");

			if (ImGui::BeginPopup("CopyColourOverlayAs"))
				ColourCopyAs();

			TacitImage::ImgInfo& info = CurrImage->Info;
			if (info.IsValid())
			{
				ImGui::Text("Size: %dx%d", info.Width, info.Height);
				ImGui::Text("Pixel Format: %s", info.PixelFormat.ConstText());
				ImGui::Text("Bit Depth: %d", info.SrcFileBitDepth);
				ImGui::Text("Opaque: %s", info.Opaque ? "true" : "false");
				ImGui::Text("Mipmaps: %d", info.Mipmaps);
				ImGui::Text("File Size (B): %d", info.FileSizeBytes);
				ImGui::Text("Cursor: (%d, %d)", cursorX, cursorY);
				ImGui::Text("Zoom: %.0f%%", zoom);
			}
		}
		ImGui::Text("Images In Folder: %d", Images.GetNumItems());

		if (ImGui::BeginPopupContextWindow())
		{
			if (ImGui::MenuItem("Top-left",		nullptr, Config.OverlayCorner == 0)) Config.OverlayCorner = 0;
			if (ImGui::MenuItem("Top-right",	nullptr, Config.OverlayCorner == 1)) Config.OverlayCorner = 1;
			if (ImGui::MenuItem("Bottom-left",  nullptr, Config.OverlayCorner == 2)) Config.OverlayCorner = 2;
			if (ImGui::MenuItem("Bottom-right", nullptr, Config.OverlayCorner == 3)) Config.OverlayCorner = 3;
			if (popen && ImGui::MenuItem("Close")) *popen = false;
			ImGui::EndPopup();
		}
	}
	ImGui::End();
}


void TexView::ColourCopyAs()
{
	tColourf floatCol(PixelColour);
	ImGui::Text("Copy As...");
	int ri = PixelColour.R; int gi = PixelColour.G; int bi = PixelColour.B; int ai = PixelColour.A;
	float rf = floatCol.R; float gf = floatCol.G; float bf = floatCol.B; float af = floatCol.A;
	tString cpyTxt;
	tsPrintf(cpyTxt, "%02X%02X%02X%02X", ri, gi, bi, ai);
	if (ImGui::Selectable(cpyTxt.ConstText()))
		ImGui::SetClipboardText(cpyTxt.ConstText());
	tsPrintf(cpyTxt, "%02X%02X%02X", ri, gi, bi);
	if (ImGui::Selectable(cpyTxt.ConstText()))
		ImGui::SetClipboardText(cpyTxt.ConstText());
	tsPrintf(cpyTxt, "#%02X%02X%02X%02X", ri, gi, bi, ai);
	if (ImGui::Selectable(cpyTxt.ConstText()))
		ImGui::SetClipboardText(cpyTxt.ConstText());
	tsPrintf(cpyTxt, "#%02X%02X%02X", ri, gi, bi);
	if (ImGui::Selectable(cpyTxt.ConstText()))
		ImGui::SetClipboardText(cpyTxt.ConstText());
	tsPrintf(cpyTxt, "0x%02X%02X%02X%02X", ri, gi, bi, ai);
	if (ImGui::Selectable(cpyTxt.ConstText()))
		ImGui::SetClipboardText(cpyTxt.ConstText());
	tsPrintf(cpyTxt, "%.3f, %.3f, %.3f, %.3f", rf, gf, bf, af);
	if (ImGui::Selectable(cpyTxt.ConstText()))
		ImGui::SetClipboardText(cpyTxt.ConstText());
	tsPrintf(cpyTxt, "%.3ff, %.3ff, %.3ff, %.3ff", rf, gf, bf, af);
	if (ImGui::Selectable(cpyTxt.ConstText()))
		ImGui::SetClipboardText(cpyTxt.ConstText());
	tsPrintf(cpyTxt, "(%.3f, %.3f, %.3f, %.3f)", rf, gf, bf, af);
	if (ImGui::Selectable(cpyTxt.ConstText()))
		ImGui::SetClipboardText(cpyTxt.ConstText());
	tsPrintf(cpyTxt, "(%.3ff, %.3ff, %.3ff, %.3ff)", rf, gf, bf, af);
	if (ImGui::Selectable(cpyTxt.ConstText()))
		ImGui::SetClipboardText(cpyTxt.ConstText());
	ImGui::EndPopup();
}


void TexView::ShowCheatSheetPopup(bool* popen)
{
	tVector2 windowPos = GetDialogOrigin(1);
	ImGui::SetNextWindowPos(windowPos, ImGuiCond_FirstUseEver);
	// ImGui::SetNextWindowBgAlpha(0.6f);
	ImGuiWindowFlags flags = ImGuiWindowFlags_NoResize |
		ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings |
		ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNav;

	if (ImGui::Begin("Cheat Sheet", popen, flags))
	{
		float col = ImGui::GetCursorPosX() + 80.0f;
		ImGui::Text("Left Arrow");	ImGui::SameLine(); ImGui::SetCursorPosX(col); ImGui::Text("Previous Image");
		ImGui::Text("Right Arrow");	ImGui::SameLine(); ImGui::SetCursorPosX(col); ImGui::Text("Next Image");
		ImGui::Text("Ctrl-Left");	ImGui::SameLine(); ImGui::SetCursorPosX(col); ImGui::Text("Skip to First Image");
		ImGui::Text("Ctrl-Right");	ImGui::SameLine(); ImGui::SetCursorPosX(col); ImGui::Text("Skip to Last Image");
		ImGui::Text("Space");		ImGui::SameLine(); ImGui::SetCursorPosX(col); ImGui::Text("Next Image");
		ImGui::Text("Ctrl +");		ImGui::SameLine(); ImGui::SetCursorPosX(col); ImGui::Text("Zoom In");
		ImGui::Text("Ctrl -");		ImGui::SameLine(); ImGui::SetCursorPosX(col); ImGui::Text("Zoom Out");
		ImGui::Text("F1");			ImGui::SameLine(); ImGui::SetCursorPosX(col); ImGui::Text("Toggle Cheat Sheet");
		ImGui::Text("F11");			ImGui::SameLine(); ImGui::SetCursorPosX(col); ImGui::Text("Toggle Fullscreen");
		ImGui::Text("Alt-Enter");   ImGui::SameLine(); ImGui::SetCursorPosX(col); ImGui::Text("Toggle Fullscreen");
		ImGui::Text("Esc");			ImGui::SameLine(); ImGui::SetCursorPosX(col); ImGui::Text("Exit Fullscreen");
		ImGui::Text("Tab");			ImGui::SameLine(); ImGui::SetCursorPosX(col); ImGui::Text("Open Explorer Window");
		ImGui::Text("Delete");		ImGui::SameLine(); ImGui::SetCursorPosX(col); ImGui::Text("Delete Current Image");
		ImGui::Text("Shift-Delete");ImGui::SameLine(); ImGui::SetCursorPosX(col); ImGui::Text("Delete Current Image Permanently.");
		ImGui::Text("LMB-Click");	ImGui::SameLine(); ImGui::SetCursorPosX(col); ImGui::Text("Set Colour Reticle Pos");
		ImGui::Text("RMB-Drag");	ImGui::SameLine(); ImGui::SetCursorPosX(col); ImGui::Text("Pan Image");
		ImGui::Text("Alt-F4");		ImGui::SameLine(); ImGui::SetCursorPosX(col); ImGui::Text("Quit");
		ImGui::Text("Ctrl-S");		ImGui::SameLine(); ImGui::SetCursorPosX(col); ImGui::Text("Save As...");
		ImGui::Text("Alt-S");		ImGui::SameLine(); ImGui::SetCursorPosX(col); ImGui::Text("Save All...");
		ImGui::Text("I");			ImGui::SameLine(); ImGui::SetCursorPosX(col); ImGui::Text("Toggle Info Overlay");
		ImGui::Text("T");			ImGui::SameLine(); ImGui::SetCursorPosX(col); ImGui::Text("Toggle Tile");
		ImGui::Text("L");			ImGui::SameLine(); ImGui::SetCursorPosX(col); ImGui::Text("Toggle Log");
		ImGui::Text("F");			ImGui::SameLine(); ImGui::SetCursorPosX(col); ImGui::Text("Zoom Fit");
		ImGui::Text("D");			ImGui::SameLine(); ImGui::SetCursorPosX(col); ImGui::Text("Zoom Downscale Only");
		ImGui::Text("Z");			ImGui::SameLine(); ImGui::SetCursorPosX(col); ImGui::Text("Zoom 1:1 Pixels");
		ImGui::Text("C");			ImGui::SameLine(); ImGui::SetCursorPosX(col); ImGui::Text("Contact Sheet...");
		ImGui::Text("P");			ImGui::SameLine(); ImGui::SetCursorPosX(col); ImGui::Text("Preferences...");
		ImGui::Text("V");			ImGui::SameLine(); ImGui::SetCursorPosX(col); ImGui::Text("Content Thumbnail View...");
	}
	ImGui::End();
}


void TexView::ShowAboutPopup(bool* popen)
{
	tVector2 windowPos = GetDialogOrigin(4);
	ImGui::SetNextWindowPos(windowPos, ImGuiCond_FirstUseEver);
	// ImGui::SetNextWindowBgAlpha(0.6f);
	ImGuiWindowFlags flags =
		ImGuiWindowFlags_NoResize			|	ImGuiWindowFlags_AlwaysAutoResize	|
		ImGuiWindowFlags_NoSavedSettings	|	ImGuiWindowFlags_NoFocusOnAppearing	|
		ImGuiWindowFlags_NoNav;

	if (ImGui::Begin("About", popen, flags))
	{
		int glfwMajor = 0; int glfwMinor = 0; int glfwRev = 0;
		glfwGetVersion(&glfwMajor, &glfwMinor, &glfwRev);
		ImGui::Text("Tacit Viewer V %d.%d.%d by Tristan Grimmer", TexView::MajorVersion, TexView::MinorVersion, TexView::Revision);
		ImGui::Separator();
		ImGui::Text("The following amazing and liberally licenced frameworks are used by this tool.");
		ImGui::Text("Dear ImGui V %s", IMGUI_VERSION);
		ImGui::Text("GLEW V %s", glewGetString(GLEW_VERSION));
		ImGui::Text("GLFW V %d.%d.%d", glfwMajor, glfwMinor, glfwRev);
		ImGui::Text("Tacent Library V %d.%d.%d", tVersion::Major, tVersion::Minor, tVersion::Revision);
		ImGui::Text("CxImage");
		ImGui::Text("nVidia Texture Tools");
		ImGui::Text("Ionicons");
		ImGui::Text("Roboto Google Font");
	}
	ImGui::End();
}


void TexView::ShowPreferencesDialog(bool* popen)
{
	ImGuiWindowFlags windowFlags = ImGuiWindowFlags_AlwaysAutoResize;

	// We specify a default position/size in case there's no data in the .ini file. Typically this isn't required! We only
	// do it to make the Demo applications a little more welcoming.
	tVector2 windowPos = GetDialogOrigin(3);
	ImGui::SetNextWindowPos(windowPos, ImGuiCond_FirstUseEver);

	if (!ImGui::Begin("Preferences", popen, windowFlags))
	{
		ImGui::End();
		return;
	}

	ImGui::Text("Background");
	ImGui::Indent();
	ImGui::Checkbox("Extend", &Config.BackgroundExtend);
	const char* backgroundItems[] = { "None", "Checkerboard", "Black", "Grey", "White" };
	ImGui::PushItemWidth(110);
	ImGui::Combo("Style", &Config.BackgroundStyle, backgroundItems, IM_ARRAYSIZE(backgroundItems));
	ImGui::PopItemWidth();
	ImGui::Unindent();

	ImGui::Separator();
	ImGui::Text("Slideshow");
	ImGui::Indent();
	ImGui::PushItemWidth(110);
	ImGui::InputDouble("Frame Duration (s)", &Config.SlidehowFrameDuration, 0.001f, 1.0f, "%.3f");
	ImGui::PopItemWidth();
	if (ImGui::Button("Reset Duration"))
		Config.SlidehowFrameDuration = 1.0/30.0;
	ImGui::Unindent();

	ImGui::Separator();
	ImGui::Text("System");
	ImGui::Indent();

	ImGui::PushItemWidth(110);

	ImGui::InputInt("Max Mem (MB)", &Config.MaxImageMemMB); ImGui::SameLine();
	ShowHelpMark("Approx memory use limit of this app. Minimum 256 MB.");
	tMath::tiClampMin(Config.MaxImageMemMB, 256);

	ImGui::InputInt("Max Cache Files", &Config.MaxCacheFiles); ImGui::SameLine();
	ShowHelpMark("Maximum number of cache files that may be created. Minimum 200.");
	tMath::tiClampMin(Config.MaxCacheFiles, 200);

	ImGui::PopItemWidth();
	ImGui::Unindent();

	ImGui::Separator();
	ImGui::Text("Interface");
	ImGui::Indent();
	ImGui::Checkbox("Confirm Deletes", &Config.ConfirmDeletes);
	ImGui::Checkbox("Confirm File Overwrites", &Config.ConfirmFileOverwrites);
	if (ImGui::Button("Reset UI"))
	{
		GLFWmonitor* monitor = glfwGetPrimaryMonitor();
		const GLFWvidmode* mode = glfwGetVideoMode(monitor);
		Config.Reset(mode->width, mode->height);
		ChangeScreenMode(false, true);
	}
	ImGui::Unindent();

	ImGui::End();
}


void TexView::DoSaveAsModalDialog(bool justOpened)
{
	static int finalWidth = 512;
	static int finalHeight = 512;
	if (justOpened && CurrImage)
	{
		finalWidth = CurrImage->GetWidth();
		finalHeight = CurrImage->GetHeight();
	}
	ImGui::InputInt("Final Width", &finalWidth); ImGui::SameLine();
	ShowHelpMark("Final scaled output width in pixels.");

	ImGui::InputInt("Final Height", &finalHeight); ImGui::SameLine();
	ShowHelpMark("Final scaled output height in pixels.");

	if (ImGui::Button("Prev Pow2"))
	{
		finalWidth = tMath::tNextLowerPower2(CurrImage->GetWidth());
		finalHeight = tMath::tNextLowerPower2(CurrImage->GetHeight());
	}
	ImGui::SameLine();
	if (ImGui::Button("Next Pow2"))
	{
		finalWidth = tMath::tNextHigherPower2(CurrImage->GetWidth());
		finalHeight = tMath::tNextHigherPower2(CurrImage->GetHeight());
	}
	ImGui::SameLine();
	if (ImGui::Button("Reset"))
	{
		finalWidth = CurrImage->GetWidth();
		finalHeight = CurrImage->GetHeight();
	}

	// Matches tImage::tPicture::tFilter.
	const char* filterItems[] = { "NearestNeighbour", "Box", "Bilinear", "Bicubic", "Quadratic", "Hamming" };
	ImGui::Combo("Filter", &Config.ResampleFilter, filterItems, IM_ARRAYSIZE(filterItems));
	ImGui::SameLine();
	ShowHelpMark("Filtering method to use when resizing images.");

	const char* fileTypeItems[] = { "tga", "png", "bmp", "jpg", "gif" };
	ImGui::Combo("File Type", &Config.FileSaveType, fileTypeItems, IM_ARRAYSIZE(fileTypeItems));
	ImGui::SameLine();
	ShowHelpMark("Output image format. JPG and GIF do not support alpha channel.");

	tString extension = ".tga";
	switch (Config.FileSaveType)
	{
		case 0: extension = ".tga"; break;
		case 1: extension = ".png"; break;
		case 2: extension = ".bmp"; break;
		case 3: extension = ".jpg"; break;
		case 4: extension = ".gif"; break;
	}

	if (Config.FileSaveType == 0)
		ImGui::Checkbox("RLE Compression", &Config.FileSaveTargaRLE);

	static char filename[128] = "Filename";
	if (justOpened)
	{
		tString baseName = tSystem::tGetFileBaseName(CurrImage->Filename);
		tStrcpy(filename, baseName.ConstText());
	}
	ImGui::InputText("Filename", filename, IM_ARRAYSIZE(filename));
	ImGui::SameLine(); ShowHelpMark("The output filename without extension.");

	ImGui::NewLine();
	if (ImGui::Button("Cancel", tVector2(100, 0)))
		ImGui::CloseCurrentPopup();
	ImGui::SameLine();
	
	ImGui::SetCursorPosX(ImGui::GetWindowContentRegionMax().x - 100.0f);

	tString imagesDir = tSystem::tGetCurrentDir();
	if (ImageFileParam.IsPresent() && tSystem::tIsAbsolutePath(ImageFileParam.Get()))
		imagesDir = tSystem::tGetDir(ImageFileParam.Get());
	tString outFile = imagesDir + tString(filename) + extension;
	bool saved = false;
	if (ImGui::Button("Save", tVector2(100, 0)))
	{
		if (tFileExists(outFile) && Config.ConfirmFileOverwrites)
			ImGui::OpenPopup("Overwrite File");
		else
		{
			SaveImageTo(outFile, finalWidth, finalHeight);
			saved = true;
		}
	}

	if (ImGui::BeginPopupModal("Overwrite File", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
	{
		if (DoOverwriteFileModal(outFile, finalWidth, finalHeight))
			saved = true;
	}

	if (saved)
		ImGui::CloseCurrentPopup();

	ImGui::EndPopup();
}


void TexView::DoSaveAllModalDialog(bool justOpened)
{
	tString msg;
	tsPrintf
	(
		msg,
		"Saves all %d images to the image type you select.\n"
		"If the file already exists it is not overwritten.\n"
		"\n"
		"If you select force-size, all written images will\n"
		"be resized as necessary. If You do not, each file\n"
		"keeps its original size.\n"
		"\n"
		"This dialog will close when operation has completed.\n"
		"Select 'View->Show Log' to see the results.\n",
		Images.GetNumItems()
	);
	ImGui::Text(msg.ConstText());
	ImGui::Separator();

	ImGui::Text("\n");
	static int forceWidth = 512;
	static int forceHeight = 512;
	static bool forceSize = false;
	ImGui::Checkbox("Force Size", &forceSize);
	if (forceSize)
	{
		ImGui::InputInt("Forced Width", &forceWidth);
		ImGui::SameLine();
		ShowHelpMark("Output width in pixels for all images.");

		ImGui::InputInt("Forced Height", &forceHeight);
		ImGui::SameLine();
		ShowHelpMark("Output height in pixels for all images.");

		// Matches tImage::tPicture::tFilter.
		const char* filterItems[] = { "NearestNeighbour", "Box", "Bilinear", "Bicubic", "Quadratic", "Hamming" };
		ImGui::Combo("Filter", &Config.ResampleFilter, filterItems, IM_ARRAYSIZE(filterItems));
		ImGui::SameLine();
		ShowHelpMark("Filtering method to use when resizing images.");
	}
	tMath::tiClampMin(forceWidth, 4);
	tMath::tiClampMin(forceHeight, 4);

	const char* fileTypeItems[] = { "tga", "png", "bmp", "jpg", "gif" };
	ImGui::Combo("File Type", &Config.FileSaveType, fileTypeItems, IM_ARRAYSIZE(fileTypeItems));
	ImGui::SameLine();
	ShowHelpMark("Output image format. JPG and GIF do not support alpha channel.");

	tString extension = ".tga";
	switch (Config.FileSaveType)
	{
		case 0: extension = ".tga"; break;
		case 1: extension = ".png"; break;
		case 2: extension = ".bmp"; break;
		case 3: extension = ".jpg"; break;
		case 4: extension = ".gif"; break;
	}

	if (Config.FileSaveType == 0)
		ImGui::Checkbox("RLE Compression", &Config.FileSaveTargaRLE);

	ImGui::NewLine();
	if (ImGui::Button("Cancel", tVector2(100, 0)))
		ImGui::CloseCurrentPopup();

	ImGui::SameLine();	
	ImGui::SetCursorPosX(ImGui::GetWindowContentRegionMax().x - 100.0f);
	if (ImGui::Button("Save All", tVector2(100, 0)))
	{
		tString currFile = CurrImage ? CurrImage->Filename : tString();
		tString imagesDir = tSystem::tGetCurrentDir();
		if (ImageFileParam.IsPresent() && tSystem::tIsAbsolutePath(ImageFileParam.Get()))
			imagesDir = tSystem::tGetDir(ImageFileParam.Get());

		for (TacitImage* image = Images.First(); image; image = image->Next())
		{
			tString baseName = tSystem::tGetFileBaseName(image->Filename);
			tString outFile = imagesDir + tString(baseName) + extension;
			if (tSystem::tFileExists(outFile))
			{
				tPrintf("File %s exists. Will not overwrite\n", outFile.ConstText());
				continue;
			}

			// We make sure to maintain the loaded/unloaded state of all images. This function
			// can process many files, so we don't want them all in memory at once by indiscriminantly
			// loading them all.
			bool imageLoaded = image->IsLoaded();
			if (!imageLoaded)
				image->Load();
			tImage::tPicture outPic; //image->GetWidth(), image->GetHeight());
			outPic.Set(*image->GetPrimaryPicture());
			if (!imageLoaded)
				image->Unload();

			int outWidth = outPic.GetWidth();
			int outHeight = outPic.GetHeight();
			if (forceSize)
			{
				outWidth = forceWidth;
				outHeight = forceHeight;
			}

			if ((outPic.GetWidth() != outWidth) || (outPic.GetHeight() != outHeight))
				outPic.Resample(outWidth, outHeight, tImage::tPicture::tFilter(Config.ResampleFilter));

			bool success = false;
			tImage::tPicture::tColourFormat colourFmt = outPic.IsOpaque() ? tImage::tPicture::tColourFormat::Colour : tImage::tPicture::tColourFormat::ColourAndAlpha;
			if (Config.FileSaveType == 0)
				success = outPic.SaveTGA(outFile, tImage::tFileTGA::tFormat::Auto, Config.FileSaveTargaRLE ? tImage::tFileTGA::tCompression::RLE : tImage::tFileTGA::tCompression::None);
			else
				success = outPic.Save(outFile, colourFmt);

			if (success)
				tPrintf("Saved image as : %s\n", outFile.ConstText());
			else
				tPrintf("Failed to save image %s\n", outFile.ConstText());
		}

		Images.Clear();
		PopulateImages();
		SetCurrentImage(currFile);

		ImGui::CloseCurrentPopup();
	}

	ImGui::EndPopup();
}


void TexView::SaveImageTo(const tString& outFile, int finalWidth, int finalHeight)
{
	tImage::tPicture outPic(CurrImage->GetWidth(), CurrImage->GetHeight());
	outPic.Set(*CurrImage->GetPrimaryPicture());

	if ((outPic.GetWidth() != finalWidth) || (outPic.GetHeight() != finalHeight))
		outPic.Resample(finalWidth, finalHeight, tImage::tPicture::tFilter(Config.ResampleFilter));

	bool success = false;
	tImage::tPicture::tColourFormat colourFmt = outPic.IsOpaque() ? tImage::tPicture::tColourFormat::Colour : tImage::tPicture::tColourFormat::ColourAndAlpha;
	if (Config.FileSaveType == 0)
		success = outPic.SaveTGA(outFile, tImage::tFileTGA::tFormat::Auto, Config.FileSaveTargaRLE ? tImage::tFileTGA::tCompression::RLE : tImage::tFileTGA::tCompression::None);
	else
		success = outPic.Save(outFile, colourFmt);
	if (success)
		tPrintf("Saved image as : %s\n", outFile.ConstText());
	else
		tPrintf("Failed to save image %s\n", outFile.ConstText());

	Images.Clear();
	PopulateImages();
	SetCurrentImage(outFile);
}


bool TexView::DoOverwriteFileModal(const tString& outFile, int finalWidth, int finalHeight)
{
	bool saved = false;
	tString file = tSystem::tGetFileName(outFile);
	tString dir = tSystem::tGetDir(outFile);
	ImGui::Text("Overwrite file");
		ImGui::Indent(); ImGui::Text("%s", file.ConstText()); ImGui::Unindent();
	ImGui::Text("In Folder");
		ImGui::Indent(); ImGui::Text("%s", dir.ConstText()); ImGui::Unindent();
	ImGui::NewLine();
	ImGui::Separator();
			
	ImGui::NewLine();
	ImGui::Checkbox("Confirm file overwrites in the future?", &Config.ConfirmFileOverwrites);
	ImGui::NewLine();

	if (ImGui::Button("Cancel", tVector2(100, 0)))
		ImGui::CloseCurrentPopup();

	ImGui::SameLine();
	ImGui::SetCursorPosX(ImGui::GetWindowContentRegionMax().x - 100.0f);
	if (ImGui::Button("OK", tVector2(100, 0)))
	{
		SaveImageTo(outFile, finalWidth, finalHeight);
		saved = true;
		ImGui::CloseCurrentPopup();
	}
	ImGui::EndPopup();
	return saved;
}


void TexView::DoDeleteFileModal()
{
	tString fullname = CurrImage->Filename;
	tString file = tSystem::tGetFileName(fullname);
	tString dir = tSystem::tGetDir(fullname);
	ImGui::Text("Delete File");
		ImGui::Indent(); ImGui::Text("%s", file.ConstText()); ImGui::Unindent();
	ImGui::Text("In Folder");
		ImGui::Indent(); ImGui::Text("%s", dir.ConstText()); ImGui::Unindent();
	ImGui::NewLine();
	ImGui::Separator();

	ImGui::NewLine();
	ImGui::Checkbox("Confirm file deletions in the future?", &Config.ConfirmDeletes);
	ImGui::NewLine();

	if (ImGui::Button("Cancel", tVector2(100, 0)))
		ImGui::CloseCurrentPopup();

	ImGui::SameLine();
	ImGui::SetCursorPosX(ImGui::GetWindowContentRegionMax().x - 100.0f);

	if (ImGui::Button("OK", tVector2(100, 0)))
	{
		DeleteImageFile(fullname, true);
		ImGui::CloseCurrentPopup();
	}
	ImGui::SetItemDefaultFocus();
	ImGui::EndPopup();
}


void TexView::DoDeleteFileNoRecycleModal()
{
	tString fullname = CurrImage->Filename;
	tString file = tSystem::tGetFileName(fullname);
	tString dir = tSystem::tGetDir(fullname);
	ImGui::Text("Delete File");
		ImGui::Indent(); ImGui::Text("%s", file.ConstText()); ImGui::Unindent();
	ImGui::Text("In Folder");
		ImGui::Indent(); ImGui::Text("%s", dir.ConstText()); ImGui::Unindent();
	ImGui::NewLine();
	ImGui::Separator();
	ImGui::NewLine();

	ImGui::Text("This operation cannot be undone. The file\nwill be deleted permanently.");
	ImGui::NewLine();

	if (ImGui::Button("Cancel", tVector2(100, 0)))
		ImGui::CloseCurrentPopup();

	ImGui::SetItemDefaultFocus();
	ImGui::SameLine();
	ImGui::SetCursorPosX(ImGui::GetWindowContentRegionMax().x - 100.0f);

	if (ImGui::Button("OK", tVector2(100, 0)))
	{
		DeleteImageFile(fullname, false);
		ImGui::CloseCurrentPopup();
	}

	ImGui::EndPopup();
}
