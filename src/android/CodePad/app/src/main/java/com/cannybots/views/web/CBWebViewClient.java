package com.cannybots.views.web;

import android.webkit.WebView;
import android.webkit.WebViewClient;

/**
 * Created by wayne on 26/01/15.
 */
public class CBWebViewClient extends WebViewClient {
    public boolean shouldOverrideUrlLoading(WebView v, String url)
    {
        v.loadUrl(url);
        return true;
    }

}
