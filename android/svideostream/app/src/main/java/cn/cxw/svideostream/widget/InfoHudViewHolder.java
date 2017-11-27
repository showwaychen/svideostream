package cn.cxw.svideostream.widget;

import android.content.Context;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.TableLayout;
import android.widget.TextView;

import cn.cxw.svideostream.R;
import cn.cxw.svideostreamlib.StatsReport;

/**
 * Created by user on 2017/11/24.
 */

public class InfoHudViewHolder {
    TableLayout mInfoView;
    Context mContext = null;
    int mCurRowNum = 0;
    public InfoHudViewHolder(Context context, TableLayout tlinfo)
    {
        mContext = context;
        mInfoView = tlinfo;
    }

    public void updateInfo(StatsReport[] reports)
    {
        mInfoView.removeAllViews();
        if (reports == null)
        {
            return ;
        }
        int rows = 0;
        for (int i = 0; i < reports.length; i++)
        {
            rows += reports[i].mValues.length;
        }
        if (rows == 0)
        {
            return ;
        }
        for (int i = 0; i < reports.length; i++)
        {

            for (int j = 0; j < reports[i].mValues.length;j ++)
            {
                ViewGroup rowView = (ViewGroup) LayoutInflater.from(mContext).inflate(R.layout.table_stream_info_row, mInfoView, false);
                setNameValueText(rowView, reports[i].mValues[j].name, reports[i].mValues[j].value);
                mInfoView.addView(rowView);
            }
        }

    }

    private static class ViewHolder {
        public TextView mNameTextView;
        public TextView mValueTextView;

        public void setName(String name) {
            if (mNameTextView != null) {
                mNameTextView.setText(name);
            }
        }

        public void setValue(String value) {
            if (mValueTextView != null) {
                mValueTextView.setText(value);
            }
        }
    }
    public ViewHolder obtainViewHolder(View rowView) {
        ViewHolder viewHolder = (ViewHolder) rowView.getTag();
        if (viewHolder == null) {
            viewHolder = new ViewHolder();
            viewHolder.mNameTextView = (TextView) rowView.findViewById(R.id.name);
            viewHolder.mValueTextView = (TextView) rowView.findViewById(R.id.value);
            rowView.setTag(viewHolder);
        }
        return viewHolder;
    }
    public void setNameValueText(View rowView, String name, String value) {
        ViewHolder viewHolder = obtainViewHolder(rowView);
        viewHolder.setName(name);
        viewHolder.setValue(value);
    }
}
