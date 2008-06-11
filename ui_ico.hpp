
#define ICOSIZE_16 16
#define ICOSIZE_32 32

HICON sUIIcon::getIcon() {return ico?ico:(index[0]!=-1)?ico=ImageList_ExtractIcon(0 ,iml[0] , index[0]):0;
                  }

int sUIIcon::draw(IML_enum what , HDC hDC , int x , int y , int style , int state , int rop , int frame , COLORREF effect) {
    if ((what & (IML_16|IML_32)) && (index[0]!=-1 || index[1]!=-1) && iml) {
        int i = (what & IML_16 && index[0]!=-1)?0 : (what & IML_32 && index[1]!=-1)?1 : -1;
        if (i==-1) return 0;
        IMAGELISTDRAWPARAMS ildp;
        ildp.cbSize=sizeof(IMAGELISTDRAWPARAMS);
        ildp.himl=iml[i];
        ildp.i=index[i];
        ildp.hdcDst=hDC;
//      ImageList_GetIconSize(iml[imList] , &ildp.x , &ildp.y);
        ildp.x=x;
        ildp.y=y;
        ildp.cx=ildp.cy=ildp.xBitmap=ildp.yBitmap=0;
        ildp.rgbBk=CLR_NONE;
        ildp.rgbFg=CLR_NONE;
        ildp.fStyle=style | ILD_TRANSPARENT;
        ildp.fState=state;
        ildp.dwRop=rop;
        ildp.Frame=frame;
        ildp.crEffect=0x00FF0000;
        if (!ImageList_DrawIndirect(&ildp))
            ImageList_Draw(iml[i],index[i],hDC , x,y,0);
    } else if (what & IML_ICO|IML_ICO2 && (ico||ico_)) {
        HICON ic = (what & IML_ICO && ico)?ico : (what & IML_ICO2 && ico_)?ico_ : 0;
        if (!ic) return 0;
        DrawIconEx(hDC , x , y ,
                        ic , 0,0,0,0,DI_NORMAL);
    } else if (what & IML_BMP && bmp) {
        DrawBitmap(hDC , bmp , x , y);
    }
    return 1;
 }
 int sUIIcon::draw(HDC hDC , int x , int y , int style , int state , int rop , int frame , COLORREF effect) {
     return draw(getType() , hDC , x , y , style , state , rop , frame , effect);
 }

 int sUIIcon::measure(IML_enum what , int * w , int * h) {
    *w = 0; *h=0;
    if (what & IML_16 && index[0]!=-1 && iml) {
      ImageList_GetIconSize(iml[0] , w , h);
    } else
    if (what & IML_32 && index[1]!=-1 && iml) {
      ImageList_GetIconSize(iml[1] , w , h);
    } else
    if (what & IML_ICO|IML_ICO2 && (ico||ico_)) {
        HICON ic = (what & IML_ICO && ico)?ico : (what & IML_ICO2 && ico_)?ico_ : 0;
        if (!ic) {return 0;}
        ICONINFO ii;
        if (!GetIconInfo(ic , &ii)) return 0;
        BITMAP b;
        GetObject(ii.hbmColor , sizeof(BITMAP),&b);
        *w=b.bmWidth;
        *h=b.bmHeight;
		DeleteObject(ii.hbmColor);
		DeleteObject(ii.hbmMask);
    } else if (what & IML_BMP && bmp) {
          BITMAP cbmp;
          GetObject(bmp , sizeof(BITMAP),&cbmp);
          *w=cbmp.bmWidth;
          *h=cbmp.bmHeight;
    }
    return 0;
 }
 int sUIIcon::measure(int *w , int *h) {
     return measure(getType() , w , h);
 }
 IML_enum sUIIcon::getType(){
      int type = IML_NONE;
      if (index[0]!=-1)
          type |= IML_16;
      if (index[1]!=-1)
          type |= IML_32;
      if (ico)
          type |= IML_ICO;
      if (ico_)
          type |= IML_ICO2;
      if (bmp)
          type |= IML_BMP;
      return (IML_enum)type;
  }

  sUIIcon::sUIIcon() {
    bmp=0;ico=ico_=0;isIcon=0;memset(index , -1 , sizeof(index));iml=0;
  }
  sUIIcon::~sUIIcon() {
    if (bmp) DeleteObject(bmp);
    if (ico) DestroyIcon(ico);
    if (ico_) DestroyIcon(ico_);
  }



  cUIIcons::cUIIcons() {
    iml[0]=ImageList_Create(ICOSIZE_16,ICOSIZE_16,/*ILC_COLORDDB|*/ILC_COLOR32|ILC_MASK ,30,1);
    ImageList_SetBkColor(iml[0] , CLR_NONE);
    iml[1]=ImageList_Create(ICOSIZE_32,ICOSIZE_32,/*ILC_COLORDDB|*/ILC_COLOR32|ILC_MASK ,5,1);
    ImageList_SetBkColor(iml[1] , CLR_NONE);
    Ico[0].iml = iml;
  }

  cUIIcons::~cUIIcons() {
    ImageList_Destroy(iml[0]);
    ImageList_Destroy(iml[1]);
  }           
  int cUIIcons::iconRes(int id , int res , HINSTANCE inst , IML_enum target , sUIIconRegister::tIconParams * ip) {
	  return this->iconRes(id, MAKEINTRESOURCE(res), inst, target, ip);
  }
  int cUIIcons::iconRes(int id , const char * res , HINSTANCE inst , IML_enum target , sUIIconRegister::tIconParams * ip) {
        if (!id) return 0;
		if (!res) res=MAKEINTRESOURCE(id);
		HICON ico=LoadIconEx((inst ? inst: Ctrl->hDll()) , res,ip ? ip->size : (target&IML_32?ICOSIZE_32:target&IML_16?ICOSIZE_16:0) , ip?ip->bits:0);
		if (!ico) return 0;
        sUIIcon & _ico = Ico[id];

        //  Ico[id].ico = hico;
        if (target & IML_32)
          _ico.index[1]=ImageList_ReplaceIcon(iml[1] , _ico.index[1] , ico);
		if (target & IML_16) { 
			if (target & IML_32 && !ip) {
				DestroyIcon(ico);
				ico=LoadIconEx((inst ? inst: Ctrl->hDll()) , res, 16 , 0);
				if (!ico) return 0;
			}
            _ico.index[0]=ImageList_ReplaceIcon(iml[0] , _ico.index[0] , ico);
		}
        if (!(/*trayBitCount && */(UIIcon_type(id) ==IT_STATUS || UIIcon_type(id)==IT_MESSAGE))) { // ikonka nie statusem/wiadomoscia przy "z³ym" systemie
            if ((target & IML_ICO)) {
              if (_ico.ico) DestroyIcon(_ico.ico);
              _ico.ico=DuplicateIcon(0 , ico);
            }
        } else {
            // na status
            if (_ico.ico_) DestroyIcon(_ico.ico_);
            _ico.ico_=LoadIconEx((inst ? inst: Ctrl->hDll()) , MAKEINTRESOURCE(res) , 16 , trayBitCount);
        }
        DestroyIcon(ico);
        _ico.iml=iml;
        return id;
  }
  int cUIIcons::bmpRes(int id , int res , HINSTANCE inst , IML_enum target) {
        if (!res) res=id;
        HBITMAP hbmp=(HBITMAP)LoadImage(inst?inst : Ctrl->hDll(),(char*) res, IMAGE_BITMAP , 0,0,LR_LOADTRANSPARENT|LR_SHARED);
        if (target & IML_16) 
          Ico[id].index[0]=ImageList_AddMasked(iml[0] , hbmp , COLOR_WINDOW);
        if (target & IML_32) 
          Ico[id].index[1]=ImageList_AddMasked(iml[1] , hbmp , COLOR_WINDOW);
        Ico[id].iml=iml;
        if (target & IML_BMP)
            Ico[id].bmp = hbmp;
        else
            DeleteObject(hbmp);
        return id;
        }
  int cUIIcons::erase (int id)
    {int i=0; while (this->find(id | i)) { Ico.erase(id|i);i++;}
    return 1;
    }

  int cUIIcons::count (int id)
    {int i=0; while (this->find(id | i)) {i++;}
      return i;
    }

  sUIIcon & cUIIcons::operator [] (int id) {
    if (ISICON_BLANK(id)) return Ico[0]; else
//    if (!find(id)) IMLOG("____%x______________",id);
    return Ico[id];
  }

  bool cUIIcons::find(int id) {
    if (ISICON_BLANK(id)) return 1;
    Ico_it_t Ico_it = Ico.find(id);
    return Ico_it!=Ico.end();
  }

  bool cUIIcons::iconAdd(IML_enum target , string URL) {
        return true;
  }

/**
    URL
    file://sciezka.typ
    res://inst/id.typ   inst==0 (ui.dll)  inst==dll (dll)
    handle://handle.typ
	reg://lista/id.typ  lista = IML16|IML32

*/

  HANDLE cUIIcons::getImageFromURL(string URL , int & _imgType , HINSTANCE dll , sUIIconRegister::tIconParams * _ip) {
	  sUIIconRegister::tIconParams ip = {0,0};
	  if (_ip) ip = *_ip;
	  RegEx r;
        _imgType=0;
		if (r.match("|^([a-z]+)://(.+)\\.([^.#]*)(?:#(.+))?$|" , URL.c_str())<=0)
            return 0;
        int imageType = 0;
        HANDLE imageHandle = NULL;
#define IMAGE_OTHER 3
		imageType = (r[3]=="bmp"?IMAGE_BITMAP:r[3]=="ico"?IMAGE_ICON:IMAGE_OTHER);
		CStdString params = "&" + r[4];
		if (imageType == IMAGE_ICON) {
			if (!ip.size && ActParamExists(params, "&size=")) 
				ip.size = atoi(GetActParam(params, "&size=").c_str());
			if (!ip.bits && ActParamExists(params, "&bits=")) 
				ip.bits = atoi(GetActParam(params, "&bits=").c_str());
		}
        if (r[1]=="file") {
			CStdString filename = ExpandEnvironmentStrings( (r[2]+"."+r[3]).c_str() );
			filename.Replace("/", "\\");
			
			if (_access(filename , 0)!=0) {
				return 0;
			} else if (imageType == IMAGE_ICON) 
				imageHandle = LoadIconEx(0 , filename , ip.size , ip.bits);
			else if (imageType == IMAGE_BITMAP)
	            imageHandle = LoadImage(0 , filename , imageType,
		            0,0,LR_LOADFROMFILE);
			else {
	            ILuint imgID;
				ilInit();
		        ilGenImages(1, &imgID);
			    ilBindImage(imgID);
				ilEnable(IL_FORMAT_SET); 
				ilSetInteger(IL_FORMAT_MODE, IL_RGBA);
				ilLoadImage((ILstring)(filename).c_str());
				imageHandle = (HANDLE)imgID;
			}

        } else if (r[1]=="res") {
            r.match("|^([0-9dlui]+)/(.+)$|" , r[2].c_str());
			HINSTANCE inst = (r[1]=="0" || r[1]=="ui")?Ctrl->hDll():r[1]=="dll"?dll:(HINSTANCE)atoi(r[1].c_str());
			// Wy³apujemy z RES nazwê typu i identyfikator
			r.match("|^(?:([^/]+)/)?([^/]+)$|" , r[2].c_str());
			CStdString type = r[1].c_str();
			CStdString id = r[2].c_str();
			r.setPattern("/^(?:\\d+)|(?:0x[0-9a-f]+)$/");
			r.setSubject(id);
			const char * idId = (r.match()>0) ? MAKEINTRESOURCE(chtoint(id)) : id;
			if (imageType == IMAGE_ICON) 
				imageHandle = LoadIconEx(inst , idId , ip.size , ip.bits);
			else if (imageType == IMAGE_BITMAP)
	            imageHandle = LoadImage(inst , idId , imageType,0,0,0);
			else {
				r.setSubject(type);
				const char * typeId = (r.match()>0) ? MAKEINTRESOURCE(chtoint(type)) : type;
	            ILuint imgID;
				ilInit();
		        ilGenImages(1, &imgID);
			    ilBindImage(imgID);
				ilEnable(IL_FORMAT_SET); 
				ilSetInteger(IL_FORMAT_MODE, IL_RGBA);
				int dataSize=0;
				void * data = LoadBinaryData(inst, idId, typeId , &dataSize);
				if (data) {
					int ilType = IL_TYPE_UNKNOWN;
					ilLoadL(ilType, data, dataSize);
				//ilutLoadResource(inst , atoi(r[2].c_str()) , (ILstring)r[3].c_str() , IL_TYPE_UNKNOWN);
					imageHandle = (HANDLE)imgID;
				}
			}
        } else if (r[1]=="handle") {
			if (imageType == IMAGE_OTHER) return 0;
            imageHandle = CopyImage((HANDLE)atoi(r[2].c_str()) , imageType , 0 , 0 , 0);
        } else if (r[1]=="reg") {
            r.match("|^([^/]+)/([0-9]+)$|" , r[2].c_str());
            int id = atoi(r[2].c_str());
            if (::Ico.find(id)) {
                if (r[1]=="IML16" || r[1]=="IML_16") {
                    imageType = IMAGE_ICON;
                    imageHandle = CopyImage(::Ico[id].getIcon() , imageType , 0 , 0 , 0);
                }
                if (r[1]=="IML32" || r[1]=="IML_32") {
                    imageType = IMAGE_ICON;
					if (::Ico[id].index[1] != -1)
						imageHandle = ImageList_ExtractIcon(0 , ::Ico[id].iml[1] , ::Ico[id].index[1]);
                }
            }
        }
		if (imageType == IMAGE_OTHER) {
			// Konwersja...
            ILuint imgID = (ILuint)imageHandle;
			int width = ilGetInteger(IL_IMAGE_WIDTH);
			int height = ilGetInteger(IL_IMAGE_HEIGHT);
			ICONINFO ii = {0};
			ii.fIcon = true;
			ii.hbmMask = 0;
			
			// pusta bitmapa jako maska...
			//ii.hbmMask = CreateBitmap(width,height,1,1,NULL);
			if (width > 64 && height > 32 || isComCtl6) {
				void * bmpData;
				BITMAPV5HEADER bi;
				ZeroMemory(&bi,sizeof(BITMAPV5HEADER));
				bi.bV5Size = sizeof(BITMAPV5HEADER);
				bi.bV5Width = width;
				bi.bV5Height = height;
				bi.bV5Planes = 1;
				bi.bV5BitCount = 32;
				bi.bV5Compression = BI_BITFIELDS;
				// The following mask specification specifies a supported 32 BPP
				// alpha format for Windows XP.
				bi.bV5RedMask   =  0x00FF0000;
				bi.bV5GreenMask =  0x0000FF00;
				bi.bV5BlueMask  =  0x000000FF;
				bi.bV5AlphaMask =  0xFF000000; 
				HDC hdc;
				hdc = GetDC(NULL);
				// Create the DIB section with an alpha channel.
				ii.hbmColor = CreateDIBSection(hdc, (BITMAPINFO *)&bi, DIB_RGB_COLORS, 
					(void **)&bmpData, NULL, (DWORD)0);
				//memcpy(bmpData , ilGetData() , width * height * 4);
				ilEnable(IL_ORIGIN_SET);
				ilOriginFunc(IL_ORIGIN_LOWER_LEFT);
				ilCopyPixels(0,0,0,width,height,1,IL_BGRA,IL_UNSIGNED_BYTE,bmpData);
				ReleaseDC(0 , hdc);
				imageHandle = ii.hbmColor;
				imageType = IMAGE_BITMAP;
			} else {
				if (width > 64 && height > 32) { /* bitmapa bez przezroczystoœci */
					IMDEBUG(DBG_MISC , "getImgFromURL uneven %dx%d" , width , height);
					ilConvertImage(IL_RGB , IL_BYTE);
					HDC hdc = GetDC(NULL);
					ii.hbmColor = ilutConvertToHBitmap(hdc);
					ReleaseDC(0 , hdc);
					imageHandle = ii.hbmColor;
					imageType = IMAGE_BITMAP;
				} else {
				// Dla systemów nie-XPkowych trzeba to zrobiæ inaczej...
					// Wczytujemy obrazek...
					ii.hbmMask = CreateBitmap(width,height,1,1,NULL);
					//IMDEBUG(DBG_MISC , "CreateMaskBitmap = %x" , ii.hbmMask);
					ILubyte * alpha = ilGetAlpha(IL_UNSIGNED_BYTE);
					ilConvertImage(IL_RGB , IL_BYTE);
					HDC hdc = GetDC(NULL);
					ii.hbmColor = ilutConvertToHBitmap(hdc);
//					IMDEBUG(DBG_MISC , "ilutConvertToHBitmap = %x" , ii.hbmColor);
					{
						//BITMAP b;
						//GetObject(ii.hbmColor , sizeof(BITMAP),&b);
//						IMDEBUG(DBG_DUMP , "BitmapInfo - bits=%d , bitsPixel=%d , width=%d , height=%d , planes=%d , type=%d , widthBytes=%d , o_width=%d , o_height=%d" , b.bmBits , b.bmBitsPixel , b.bmWidth , b.bmHeight , b.bmPlanes , b.bmType , b.bmWidthBytes , width , height);
					}
	//				memset(alpha , 0xFF , width*height);
					// Wype³niamy maskê
					HDC dcMask = CreateCompatibleDC(0);
					HDC dcColor = CreateCompatibleDC(0);
					HBITMAP o_hbmMask = (HBITMAP) SelectObject(dcMask , ii.hbmMask);
					HBITMAP o_hbmColor = (HBITMAP) SelectObject(dcColor , ii.hbmColor);
	//				FillRect(cdMask , &rc , 
					for (int y = 0; y < height; y++)
						for (int x = 0; x < width; x++) 
						{
		//					if (((char*)bmpData)[(y*width+x)*4+1] == 0)
	//						if ((pixel[(height - 1 - y)*width+x]&0xFF000000) < 0x80000000)
							if (alpha[y*width+x] < 0x80) {
								SetPixel(dcMask , x , y , 0xFFFFFF);
								SetPixel(dcColor , x , y , 0x0);
							}
							else 
								SetPixel(dcMask , x , y , 0);
	//						pixel[(height - 1 - y)*width + x]&=0xFFFFFF;
						}
					
					SelectObject(dcMask , o_hbmMask);
					SelectObject(dcColor , o_hbmColor);
					DeleteDC(dcMask);
					DeleteDC(dcColor);
					ReleaseDC(0 , hdc);
					// Trochê oszukamy, ale nie wiem jak inaczej dobraæ siê do iFree
					ilutFreePaddedData(alpha);
					imageHandle = CreateIconIndirect(&ii);
//					IMDEBUG(DBG_MISC , "CreateIconIndirect = %x , E=%d" , imageHandle , GetLastError());
					imageType = IMAGE_ICON;

					DeleteObject(ii.hbmColor);
					DeleteObject(ii.hbmMask);
				}
			}
			ilDeleteImages(1, &imgID);
		}
        _imgType=imageType;
        return imageHandle;
  }

HBITMAP makeBitmapMask(HBITMAP bmp , COLORREF color=-1) {
    if (color==-1) {
        HDC hDC = CreateCompatibleDC(0);
        SelectObject(hDC , bmp);
        color = GetPixel(hDC , 0 , 0);
        DeleteDC(hDC);
    }
    BITMAP bmpNfo;
    if (!GetObject(bmp , sizeof(BITMAP) , &bmpNfo)) return 0;
    HBITMAP mask = CreateBitmap(bmpNfo.bmWidth , bmpNfo.bmHeight , 1 , 1 , 0);
//    HBITMAP mask = (HBITMAP)CopyImage(bmp , IMAGE_BITMAP , 0 , 0 , 0);

    HDC hDCS = CreateCompatibleDC(0);
    SelectObject(hDCS , bmp);
    HDC hDCD = CreateCompatibleDC(0);
    SelectObject(hDCD , mask);

    for (int x=0; x<bmpNfo.bmWidth; x++) {
        for (int y=0; y<bmpNfo.bmHeight; y++) {
            if (GetPixel(hDCS , x , y)!=color)
                SetPixel(hDCD , x , y , 0);
            else {
                SetPixel(hDCD , x , y , 0x00FFFFFF);
                if (!isComCtl6)
                    SetPixel(hDCS , x , y , 0x00000000);
            }
        }
    }

    DeleteDC(hDCS);
    DeleteDC(hDCD);


/*
    char buff [16*16];
    for (int i =0; i<16*16; i++) {
         buff[i]=0xAA;
    }
*/


    return mask;

}
HICON makeIconFromBitmap(HBITMAP bmp) {
    ICONINFO ii;
    ii.fIcon = true;
    ii.xHotspot = ii.yHotspot = 0;
    ii.hbmColor = bmp;
    ii.hbmMask = makeBitmapMask(bmp);
    HICON icon = CreateIconIndirect(&ii);
    DeleteObject(ii.hbmMask);
	return icon;
}


void cUIIcons::addHandleToList(sUIIcon & _ico , int type , int imlIndex , HANDLE handle , HBITMAP mask) {
    if (type == IMAGE_ICON)
        _ico.index[imlIndex]=ImageList_ReplaceIcon(iml[imlIndex] , _ico.index[imlIndex] , (HICON)handle);  
	else if (type == IMAGE_BITMAP) {                                                    
		if (_ico.index[imlIndex] == -1) {
			_ico.index[imlIndex] = ImageList_GetImageCount(iml[imlIndex]);
			ImageList_SetImageCount(iml[imlIndex] , _ico.index[imlIndex] + 1);
		}
		if (mask)
			ImageList_Replace(iml[imlIndex] , _ico.index[imlIndex] , (HBITMAP)handle , mask); 
		else {
	//                _ico.index[imlIndex]=ImageList_AddMasked(iml[imlIndex] , (HBITMAP)handle , GetPixel(hDC , 0 , 0));   
			mask = makeBitmapMask((HBITMAP)handle);
			ImageList_Replace(iml[imlIndex] , _ico.index[imlIndex] , (HBITMAP)handle , mask);   
			DeleteObject(mask);
		}
	}
	_ico.iml=iml;
}

  bool cUIIcons::iconRegister(sUIIconRegister * ir , HINSTANCE hDll) {
      HANDLE handle = 0;
      int type;
      if (!ir->ID || ir->target==IML_NONE) return false;
      if (ir->URL && *ir->URL) {
          handle = getImageFromURL(ir->URL , type , hDll , ir->maskHandle?&ir->iconParams : 0);
      } else if (ir->imgId && ir->imgType==IMAGE_BITMAP) {
          bmpRes(ir->ID , ir->imgId , ir->imgInst , ir->target);
          return true;
      } else if (ir->imgId && ir->imgType==IMAGE_ICON) {
		  iconRes(ir->ID , ir->imgId , ir->imgInst , ir->target , ir->maskHandle?&ir->iconParams : 0);
          return true;
      } else if (ir->imgHandle) {
          handle = CopyImage(ir->imgHandle , ir->imgType , 0 , 0 , 0);
          type = ir->imgType;
      } else return false;
      // Rejestrujemy
      sUIIcon & _ico = Ico[ir->ID];
      if (ir->target & IML_16) {                         
          addHandleToList(_ico , type , 0 , handle , (HBITMAP)ir->maskHandle);
      }
      if (ir->target & IML_32) {
          addHandleToList(_ico , type , 1 , handle , (HBITMAP)ir->maskHandle);
      }
      HICON iconBmp = 0;
      if (ir->target & (IML_ICO|IML_ICO2) && type==IMAGE_BITMAP && handle) {
          ICONINFO ii;
          ii.fIcon = true;
          ii.xHotspot = ii.yHotspot = 0;
          ii.hbmColor = (HBITMAP)handle;
          ii.hbmMask = ir->maskHandle?(HBITMAP)ir->maskHandle : makeBitmapMask((HBITMAP)handle);
          iconBmp = CreateIconIndirect(&ii);
          if (!ir->maskHandle) {DeleteObject(ii.hbmMask);}
      }

      if (ir->target & IML_ICO && (type==IMAGE_ICON || iconBmp)) {
		  if (_ico.ico)
			  DestroyIcon(_ico.ico);
          _ico.ico = (HICON)CopyImage(iconBmp?iconBmp:handle , IMAGE_ICON , 0 , 0 ,0);
      }
      if (ir->target & IML_ICO2 && (type==IMAGE_ICON || iconBmp)) {
		  if (_ico.ico_)
			  DestroyIcon(_ico.ico_);
          _ico.ico_ = (HICON)CopyImage(iconBmp?iconBmp:handle , IMAGE_ICON , 0 , 0 ,0);
      }
      if (ir->target & IML_BMP && type==IMAGE_BITMAP) {
          if (_ico.bmp)
			  DeleteObject(_ico.bmp);
         _ico.bmp = (HBITMAP)CopyImage(handle , IMAGE_BITMAP , 0 , 0 ,0);
      }

      if (iconBmp) DestroyIcon(iconBmp);

      if (type == IMAGE_BITMAP) DeleteObject((HBITMAP)handle);
      else DestroyIcon((HICON)handle);
      return true;
  }
HANDLE cUIIcons::iconGet(int ID , IML_enum target) {
	if (!iconExists(ID , target)) return 0;
	sUIIcon * ico = &Ico.find(ID)->second;
	switch (target) {
		case IML_16: 
			return (HANDLE)ImageList_ExtractIcon(0 ,ico->iml[0] , ico->index[0]);
		case IML_32: 
			return (HANDLE)ImageList_ExtractIcon(0 ,ico->iml[1] , ico->index[1]);
		case IML_BMP: 
			return (HANDLE)ico->bmp;
		case IML_ICO:
			return (HANDLE)ico->ico;
		case IML_ICO2:
			return (HANDLE)ico->ico_;
	}
	return 0;
}


  HBITMAP BitmapFromBitmap(HBITMAP bmpSrc , int x , int y , int w , int h) {
      HDC hDCSrc = CreateCompatibleDC(0);
      SelectObject(hDCSrc , bmpSrc);

      HDC hDCDest = CreateCompatibleDC(0);
      HDC hDC = GetDC(0);
      HBITMAP bmpDest = CreateCompatibleBitmap(hDC , w , h);
      ReleaseDC(0 , hDC);
      SelectObject(hDCDest , bmpDest);
      BitBlt(hDCDest , 0 , 0 , w , h , hDCSrc , x , y , SRCCOPY);
      //FloodFill(hDCDest , 1 , 1 , RGB(255,0,0));
      DeleteDC(hDCSrc);
      DeleteDC(hDCDest);
      return bmpDest;
  }


  bool cUIIcons::iconRegisterList(sUIIconRegisterList * irl) {
      if (!irl->count || !irl->idList || irl->target==IML_NONE) return false;
      int w = irl->w , h = 0 , srcCnt=0;
      // Sprawdzamy rozmiary
      if (irl->ImageList) {
          ImageList_GetIconSize((HIMAGELIST)irl->ImageList , &w , &h);
          srcCnt = ImageList_GetImageCount((HIMAGELIST)irl->ImageList);
      } else if (irl->imgBmp) {
          BITMAP bmp;
          if (!GetObject(irl->imgBmp , sizeof(BITMAP) , &bmp)) return false;
          h = bmp.bmHeight;
          if (!bmp.bmHeight || !bmp.bmWidth) return false;
          w = w?w:h;
          srcCnt = bmp.bmWidth / w;
      } else return false;
      
      for (int i=0; i<irl->count; i++) {
          if (i>=srcCnt) break;
          if (!irl->idList[i]) continue;
          if (irl->ImageList) {  // Kopiujemy z listy
              HICON ico = ImageList_ExtractIcon(0 , (HIMAGELIST)irl->ImageList , i);
              sUIIconRegister ir;
              ir.ID = irl->idList[i];
              ir.target = irl->target;
              ir.imgHandle = ico;
              ir.imgType = IMAGE_ICON;
              iconRegister(&ir);
              DestroyIcon(ico);
          } else { // Kopiujemy z bitmapy
              HBITMAP bmp = BitmapFromBitmap((HBITMAP)irl->imgBmp , i*w , 0 , w , h);
              HBITMAP mask = irl->maskBmp?BitmapFromBitmap((HBITMAP)irl->maskBmp , i*w , 0 , w , h):makeBitmapMask(bmp);
              sUIIconRegister ir;
              ir.ID = irl->idList[i];
              ir.target = irl->target;
              ir.imgHandle = bmp;
              ir.maskHandle = mask;
              ir.imgType = IMAGE_BITMAP;
              iconRegister(&ir);
              DeleteObject(bmp);
              if (mask) DeleteObject(mask);
          }
      }
      return true;
  }
  bool cUIIcons::iconExists(int ID , IML_enum target){
      if (Ico.find(ID)==Ico.end()) return false;
      if (target==IML_16)   
          return Ico[ID].index[0]!=-1;
      if (target==IML_32)   
          return Ico[ID].index[1]!=-1;
      if (target==IML_ICO)   
          return Ico[ID].ico!=0;
      if (target==IML_ICO2)   
          return Ico[ID].ico_!=0;
      if (target==IML_BMP)   
          return Ico[ID].bmp!=0;
      return false;
  }
  int cUIIcons::getIconIndex(int ID , IML_enum target) {
      if (Ico.find(ID)==Ico.end()) return -1;
      if (target==IML_16)   
          return Ico[ID].index[0];
      if (target==IML_32)   
          return Ico[ID].index[1];
      return -1;
  }
  HIMAGELIST cUIIcons::getImageList(IML_enum target) {
      if (target==IML_16)   
          return iml[0];
      if (target==IML_32)   
          return iml[1];
      return 0;
  }
  bool cUIIcons::iconUnregister(int ID , IML_enum target) {
      if (Ico.find(ID)==Ico.end()) return false;
	  sUIIcon & ic = Ico[ID];
	  if (target & IML_16)
		  if (ic.index[0]!=-1) {/*ImageList_Remove(iml[0] , ic.index[0]);*/ ic.index[0]=-1;}
	  if (target & IML_32)
		  if (ic.index[1]!=-1) {/*ImageList_Remove(iml[1] , ic.index[1]);*/ ic.index[1]=-1;}
	  if (target & IML_ICO)
		  if (ic.ico) {DestroyIcon(ic.ico);ic.ico = 0;}
	  if (target & IML_ICO2)
		  if (ic.ico_) {DestroyIcon(ic.ico_);ic.ico_ = 0;}
	  if (target & IML_BMP)
		  if (ic.bmp) {
			  DeleteObject(ic.bmp);
			  ic.bmp = 0;
		  }

	  return true;
  }
