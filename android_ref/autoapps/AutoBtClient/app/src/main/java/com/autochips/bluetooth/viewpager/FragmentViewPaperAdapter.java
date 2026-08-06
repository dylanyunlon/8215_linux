package com.autochips.bluetooth.viewpager;


import android.view.ViewGroup;

import androidx.fragment.app.Fragment;
import androidx.fragment.app.FragmentManager;
import androidx.fragment.app.FragmentStatePagerAdapter;

import java.util.List;


public class FragmentViewPaperAdapter extends FragmentStatePagerAdapter {
	int currentposition = -1;
	private List<Fragment> fragments;

	public FragmentViewPaperAdapter(FragmentManager fm, List<Fragment> fragments) {
		super(fm);
		this.fragments = fragments;
	}

	@Override
	public void setPrimaryItem(ViewGroup container, int position, Object object) {
		// TODO Auto-generated method stub
		setCurrentPosition(position);
		super.setPrimaryItem(container, position, object);
	}

	@Override
	public Fragment getItem(int position) {
		return fragments.get(position);
	}

	@Override
	public int getCount() {
		return fragments.size();
	}

	public void setCurrentPosition(int position) {
		this.currentposition = position;
	}

	public int getCurrentPosition() {
		return currentposition;
	}

	@Override
	public void destroyItem(ViewGroup container, int position, Object object) {

	}
}
