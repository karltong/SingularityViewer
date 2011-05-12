/** 
 * @file ascentprefssys.cpp
 * @Ascent Viewer preferences panel
 *
 * $LicenseInfo:firstyear=2008&license=viewergpl$
 * 
 * Copyright (c) 2008, Henri Beauchamp.
 * Rewritten in its entirety 2010 Hg Beeks. 
 *
 * Second Life Viewer Source Code
 * The source code in this file ("Source Code") is provided by Linden Lab
 * to you under the terms of the GNU General Public License, version 2.0
 * ("GPL"), unless you have obtained a separate licensing agreement
 * ("Other License"), formally executed by you and Linden Lab.  Terms of
 * the GPL can be found in doc/GPL-license.txt in this distribution, or
 * online at http://secondlifegrid.net/programs/open_source/licensing/gplv2
 * 
 * There are special exceptions to the terms and conditions of the GPL as
 * it is applied to this Source Code. View the full text of the exception
 * in the file doc/FLOSS-exception.txt in this software distribution, or
 * online at http://secondlifegrid.net/programs/open_source/licensing/flossexception
 * 
 * By copying, modifying or distributing this software, you acknowledge
 * that you have read and understood your obligations described above,
 * and agree to abide by those obligations.
 * 
 * ALL LINDEN LAB SOURCE CODE IS PROVIDED "AS IS." LINDEN LAB MAKES NO
 * WARRANTIES, EXPRESS, IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY,
 * COMPLETENESS OR PERFORMANCE.
 * $/LicenseInfo$
 */

#include "llviewerprecompiledheaders.h"

//File include
#include "ascentprefssys.h"
#include "llcolorswatch.h"
#include "llvoavatar.h"
#include "llhudeffectlookat.h"
#include "llagent.h"
#include "llaudioengine.h" //For POWER USER affirmation.
#include "llfloaterchat.h" //For POWER USER affirmation.
#include "llstartup.h"
#include "llviewercontrol.h"
#include "lluictrlfactory.h"
#include "llcombobox.h"
#include "llradiogroup.h"
#include "llwind.h"
#include "llviewernetwork.h"
#include "pipeline.h"
#include "lgghunspell_wrapper.h"


LLPrefsAscentSys::LLPrefsAscentSys()
{
	LLUICtrlFactory::getInstance()->buildPanel(this, "panel_preferences_ascent_system.xml");
	childSetCommitCallback("speed_rez_check", onCommitCheckBox, this);
	childSetCommitCallback("double_click_teleport_check", onCommitCheckBox, this);
	childSetCommitCallback("system_folder_check", onCommitCheckBox, this);
	childSetCommitCallback("show_look_at_check", onCommitCheckBox, this);
	childSetCommitCallback("enable_clouds", onCommitCheckBox, this);

	childSetCommitCallback("SpellBase", onSpellBaseComboBoxCommit, this);
    childSetAction("EmSpell_EditCustom", onSpellEditCustom, this);
    childSetAction("EmSpell_GetMore", onSpellGetMore, this);
    childSetAction("EmSpell_Add", onSpellAdd, this);
    childSetAction("EmSpell_Remove", onSpellRemove, this);

	childSetCommitCallback("Keywords_Alert", onCommitCheckBox, this);


	refreshValues();
	refresh();
}

LLPrefsAscentSys::~LLPrefsAscentSys()
{
}

//static
void LLPrefsAscentSys::onCommitCheckBox(LLUICtrl* ctrl, void* user_data)
{
	LLPrefsAscentSys* self = (LLPrefsAscentSys*)user_data;	
	
	llinfos << "Change to " << ctrl->getControlName()  << " aka " << ctrl->getName() << llendl;
	
	if (ctrl->getName() == "speed_rez_check")   // Why is this one getControlName() and the rest are getName()?
	{
		bool enabled = self->childGetValue("speed_rez_check").asBoolean();
		self->childSetEnabled("speed_rez_interval", enabled);
		self->childSetEnabled("speed_rez_seconds", enabled);
	}
	else if (ctrl->getName() == "show_look_at_check")
	{
		BOOL lookAt = self->childGetValue("show_look_at_check").asBoolean();
		LLHUDEffectLookAt::sDebugLookAt = lookAt;
		gSavedSettings.setBOOL("AscentShowLookAt", lookAt);
	}
	else if (ctrl->getName() == "double_click_teleport_check")
	{
		bool enabled = self->childGetValue("double_click_teleport_check").asBoolean();
		self->childSetEnabled("center_after_teleport_check", enabled);
		self->childSetEnabled("offset_teleport_check", enabled);
	}
	else if (ctrl->getName() == "system_folder_check")
	{
		bool enabled = self->childGetValue("system_folder_check").asBoolean();
		self->childSetEnabled("temp_in_system_check", enabled);
	}
	else if (ctrl->getName() == "enable_clouds")
	{
		bool enabled = self->childGetValue("enable_clouds").asBoolean();
		self->childSetEnabled("enable_classic_clouds", enabled);
	}
	else if (ctrl->getName() == "Keywords_Alert")
	{
		bool enabled = self->childGetValue("Keywords_Alert").asBoolean();
		self->childSetEnabled("Keywords_Entries", enabled);
		self->childSetEnabled("Keywords_LocalChat", enabled);
		self->childSetEnabled("Keywords_IM", enabled);
		self->childSetEnabled("Keywords_Highlight", enabled);
		self->childSetEnabled("Keywords_Color", enabled);
		self->childSetEnabled("Keywords_PlaySound", enabled);
		self->childSetEnabled("Keywords_SoundUUID", enabled);
	}
}

void LLPrefsAscentSys::onSpellAdd(void* data)
{
	LLPrefsAscentSys* self = (LLPrefsAscentSys*)data;

	if(self)
	{
		glggHunSpell->addButton(self->childGetValue("EmSpell_Avail").asString());
	}

	self->refresh();
}

void LLPrefsAscentSys::onSpellRemove(void* data)
{
	LLPrefsAscentSys* self = (LLPrefsAscentSys*)data;

	if(self)
	{
		glggHunSpell->removeButton(self->childGetValue("EmSpell_Installed").asString());
	}

	self->refresh();
}

void LLPrefsAscentSys::onSpellGetMore(void* data)
{
	glggHunSpell->getMoreButton(data);
}

void LLPrefsAscentSys::onSpellEditCustom(void* data)
{
	glggHunSpell->editCustomButton();
}

void LLPrefsAscentSys::onSpellBaseComboBoxCommit(LLUICtrl* ctrl, void* userdata)
{
	LLComboBox* box = (LLComboBox*)ctrl;

    if (box)
	{
		glggHunSpell->newDictSelection(box->getValue().asString());
	}
}

void LLPrefsAscentSys::refreshValues()
{
	//General -----------------------------------------------------------------------------
	mDoubleClickTeleport		= gSavedSettings.getBOOL("DoubleClickTeleport");
		mResetCameraAfterTP		= gSavedSettings.getBOOL("OptionRotateCamAfterLocalTP");
		mOffsetTPByUserHeight	= gSavedSettings.getBOOL("OptionOffsetTPByAgentHeight");
	mPreviewAnimInWorld			= gSavedSettings.getBOOL("PreviewAnimInWorld");
	mSaveScriptsAsMono			= gSavedSettings.getBOOL("SaveScriptsAsMono");
	mAlwaysRezInGroup			= gSavedSettings.getBOOL("AscentAlwaysRezInGroup");
	//always show Build
	mAlwaysShowFly				= gSavedSettings.getBOOL("AscentFlyAlwaysEnabled");
	//Disable camera minimum distance
	mPowerUser					= gSavedSettings.getBOOL("AscentPowerfulWizard");
	mUseSystemFolder			= gSavedSettings.getBOOL("AscentUseSystemFolder");
		mUploadToSystem				= gSavedSettings.getBOOL("AscentSystemTemporary");
	//Chat/IMs ----------------------------------------------------------------------------
	mHideNotificationsInChat	= gSavedSettings.getBOOL("HideNotificationsInChat");
	mHideTypingNotification		= gSavedSettings.getBOOL("AscentHideTypingNotification");
	mPlayTypingSound			= gSavedSettings.getBOOL("PlayTypingSound");
	mEnableMUPose				= gSavedSettings.getBOOL("AscentAllowMUpose");
	mEnableOOCAutoClose			= gSavedSettings.getBOOL("AscentAutoCloseOOC");
	mLinksForChattingObjects	= gSavedSettings.getU32("LinksForChattingObjects");
	//Time format
	//Date Format
	mSecondsInChatAndIMs		= gSavedSettings.getBOOL("SecondsInChatAndIMs");
	
	//Performance -------------------------------------------------------------------------
	mFetchInventoryOnLogin		= gSavedSettings.getBOOL("FetchInventoryOnLogin");
	mEnableLLWind				= gSavedSettings.getBOOL("WindEnabled");

	mEnableClouds				= gSavedSettings.getBOOL("CloudsEnabled");
	mEnableClassicClouds		= gSavedSettings.getBOOL("SkyUseClassicClouds");

	mSpeedRez					= gSavedSettings.getBOOL("SpeedRez");
	mSpeedRezInterval			= gSavedSettings.getU32("SpeedRezInterval");

	//Command Line ------------------------------------------------------------------------


	//Privacy -----------------------------------------------------------------------------
	mBroadcastViewerEffects		= gSavedSettings.getBOOL("BroadcastViewerEffects");
	mDisablePointAtAndBeam		= gSavedSettings.getBOOL("DisablePointAtAndBeam");
	mPrivateLookAt				= gSavedSettings.getBOOL("PrivateLookAt");
	mShowLookAt					= LLHUDEffectLookAt::sDebugLookAt;
	mRevokePermsOnStandUp		= gSavedSettings.getBOOL("RevokePermsOnStandUp");
	mDisableClickSit			= gSavedSettings.getBOOL("DisableClickSit");
	//Text Options ------------------------------------------------------------------------
    mSpellDisplay               = gSavedSettings.getBOOL("SpellDisplay");

    mKeywordsOn                 = gSavedPerAccountSettings.getBOOL("KeywordsOn");
	mKeywordsList               = gSavedPerAccountSettings.getString("KeywordsList");
	mKeywordsInChat             = gSavedPerAccountSettings.getBOOL("KeywordsInChat");
	mKeywordsInIM               = gSavedPerAccountSettings.getBOOL("KeywordsInIM");
	mKeywordsChangeColor        = gSavedPerAccountSettings.getBOOL("KeywordsChangeColor");
	mKeywordsColor              = gSavedPerAccountSettings.getColor4("KeywordsColor");
	mKeywordsPlaySound          = gSavedPerAccountSettings.getBOOL("KeywordsPlaySound");
	mKeywordsSound              = static_cast<LLUUID>(gSavedPerAccountSettings.getString("KeywordsSound"));
}

void LLPrefsAscentSys::refresh()
{	
	//General -----------------------------------------------------------------------------
	childSetValue("double_click_teleport_check",	mDoubleClickTeleport);
		childSetValue("center_after_teleport_check",	mResetCameraAfterTP);
		childSetEnabled("center_after_teleport_check",	mDoubleClickTeleport);
		childSetValue("offset_teleport_check",			mOffsetTPByUserHeight);
		childSetEnabled("offset_teleport_check",		mDoubleClickTeleport);
	childSetValue("preview_anim_in_world_check",	mPreviewAnimInWorld);
	childSetValue("save_scripts_as_mono_check",		mSaveScriptsAsMono);
	childSetValue("always_rez_in_group_check",		mAlwaysRezInGroup);
	//Disable Teleport Progress
	//Disable Logout progress
	//always show Build
	childSetValue("always_fly_check",				mAlwaysShowFly);
	//Disable camera minimum distance
	childSetValue("power_user_check",				mPowerUser);
	childSetValue("power_user_confirm_check",		mPowerUser);
	childSetValue("system_folder_check",			mUseSystemFolder);
		childSetValue("temp_in_system_check",			mUploadToSystem);
		childSetEnabled("temp_in_system_check",			mUseSystemFolder);
	//Chat --------------------------------------------------------------------------------
	childSetValue("hide_notifications_in_chat_check", mHideNotificationsInChat);
	childSetValue("play_typing_sound_check",		mPlayTypingSound);
	childSetValue("hide_typing_check",				mHideTypingNotification);
	childSetValue("seconds_in_chat_and_ims_check",	mSecondsInChatAndIMs);
	childSetValue("allow_mu_pose_check",			mEnableMUPose);
	childSetValue("close_ooc_check",				mEnableOOCAutoClose);
	LLRadioGroup* radioLinkOptions = getChild<LLRadioGroup>("objects_link");
	radioLinkOptions->selectNthItem(mLinksForChattingObjects);
	//childSetValue("objects_link",					mLinksForChattingObjects);
	std::string format = gSavedSettings.getString("ShortTimeFormat");
	if (format.find("%p") == -1)
	{
		mTimeFormat = 0;
	}
	else
	{
		mTimeFormat = 1;
	}

	format = gSavedSettings.getString("ShortDateFormat");
	if (format.find("%m/%d/%") != -1)
	{
		mDateFormat = 2;
	}
	else if (format.find("%d/%m/%") != -1)
	{
		mDateFormat = 1;
	}
	else
	{
		mDateFormat = 0;
	}

	// time format combobox
	LLComboBox* combo = getChild<LLComboBox>("time_format_combobox");
	if (combo)
	{
		combo->setCurrentByIndex(mTimeFormat);
	}

	// date format combobox
	combo = getChild<LLComboBox>("date_format_combobox");
	if (combo)
	{
		combo->setCurrentByIndex(mDateFormat);
	}

	childSetValue("seconds_in_chat_and_ims_check",	mSecondsInChatAndIMs);


	LLWString auto_response = utf8str_to_wstring( gSavedPerAccountSettings.getString("AscentInstantMessageResponse") );
	LLWStringUtil::replaceChar(auto_response, '^', '\n');
	LLWStringUtil::replaceChar(auto_response, '%', ' ');
	childSetText("im_response", wstring_to_utf8str(auto_response));
	childSetValue("AscentInstantMessageResponseFriends", gSavedPerAccountSettings.getBOOL("AscentInstantMessageResponseFriends"));
	childSetValue("AscentInstantMessageResponseMuted", gSavedPerAccountSettings.getBOOL("AscentInstantMessageResponseMuted"));
	childSetValue("AscentInstantMessageResponseAnyone", gSavedPerAccountSettings.getBOOL("AscentInstantMessageResponseAnyone"));
	childSetValue("AscentInstantMessageShowResponded", gSavedPerAccountSettings.getBOOL("AscentInstantMessageShowResponded"));
	childSetValue("AscentInstantMessageShowOnTyping", gSavedPerAccountSettings.getBOOL("AscentInstantMessageAnnounceIncoming"));
	childSetValue("AscentInstantMessageResponseRepeat", gSavedPerAccountSettings.getBOOL("AscentInstantMessageResponseRepeat" ));
	childSetValue("AscentInstantMessageResponseItem", gSavedPerAccountSettings.getBOOL("AscentInstantMessageResponseItem"));


	//Save Performance --------------------------------------------------------------------
	childSetValue("fetch_inventory_on_login_check", mFetchInventoryOnLogin);
	childSetValue("enable_wind", mEnableLLWind);
	childSetValue("enable_clouds", mEnableClouds);
		childSetValue("enable_classic_clouds", mEnableClassicClouds);
	gLLWindEnabled = mEnableLLWind;
	childSetValue("speed_rez_check", mSpeedRez);
		childSetEnabled("speed_rez_interval", mSpeedRez);
		childSetEnabled("speed_rez_seconds", mSpeedRez);
	//Command Line ------------------------------------------------------------------------

	//Privacy -----------------------------------------------------------------------------
	childSetValue("broadcast_viewer_effects", mBroadcastViewerEffects);
	childSetValue("disable_point_at_and_beams_check", mDisablePointAtAndBeam);
	childSetValue("private_look_at_check", mPrivateLookAt);
	childSetValue("show_look_at_check", mShowLookAt);
	childSetValue("revoke_perms_on_stand_up_check", mRevokePermsOnStandUp);
	childSetValue("disable_click_sit_check", mDisableClickSit);

	//Text Options ------------------------------------------------------------------------
    combo = getChild<LLComboBox>("SpellBase");

    if (combo != NULL) 
	{
		combo->removeall();
		std::vector<std::string> names = glggHunSpell->getDicts();

		for (int i = 0; i < (int)names.size(); i++) 
		{
			combo->add(names[i]);
		}

		combo->setSimple(gSavedSettings.getString("SpellBase"));
	}

    combo = getChild<LLComboBox>("EmSpell_Avail");

    if (combo != NULL) 
	{
		combo->removeall();

		combo->add("");
		std::vector<std::string> names = glggHunSpell->getAvailDicts();

		for (int i = 0; i < (int)names.size(); i++) 
		{
			combo->add(names[i]);
		}

		combo->setSimple(std::string(""));
	}

    combo = getChild<LLComboBox>("EmSpell_Installed");

    if (combo != NULL) 
	{
		combo->removeall();

		combo->add("");
		std::vector<std::string> names = glggHunSpell->getInstalledDicts();

		for (int i = 0; i < (int)names.size(); i++) 
		{
			combo->add(names[i]);
		}

		combo->setSimple(std::string(""));
	}

    childSetValue("Keywords_Alert", mKeywordsOn);
	childSetValue("Keywords_Entries", mKeywordsList);
	childSetValue("Keywords_LocalChat", mKeywordsInChat);
	childSetValue("Keywords_IM", mKeywordsInIM);
	childSetValue("Keywords_Highlight", mKeywordsChangeColor);
	childSetValue("Keywords_PlaySound", mKeywordsPlaySound);
	childSetValue("Keywords_SoundUUID", mKeywordsSound);

	LLColorSwatchCtrl* colorctrl = getChild<LLColorSwatchCtrl>("Keywords_Color");
	colorctrl->set(LLColor4(mKeywordsColor),TRUE);
}

void LLPrefsAscentSys::cancel()
{/*
	//General -----------------------------------------------------------------------------
	childSetValue("double_click_teleport_check",	mDoubleClickTeleport);
		childSetValue("center_after_teleport_check",	mResetCameraAfterTP);
		childSetValue("offset_teleport_check",			mOffsetTPByUserHeight);
	childSetValue("preview_anim_in_world_check",	mPreviewAnimInWorld);
	childSetValue("save_scripts_as_mono_check",		mSaveScriptsAsMono);
	childSetValue("always_rez_in_group_check",		mAlwaysRezInGroup);
	//Chat --------------------------------------------------------------------------------
	childSetValue("hide_notifications_in_chat_check", mHideNotificationsInChat);
	childSetValue("play_typing_sound_check",		mPlayTypingSound);
	childSetValue("hide_typing_check",				mHideTypingNotification);
	childSetValue("seconds_in_chat_and_ims_check",	mSecondsInChatAndIMs);
	childSetValue("allow_mu_pose_check",			mEnableMUPose);
	childSetValue("close_ooc_check",				mEnableOOCAutoClose);
	//Show Links
	//Time Format
	//Date Format
	childSetValue("seconds_in_chat_and_ims_check",	mEnableOOCAutoClose);
	//Save Performance --------------------------------------------------------------------
	childSetValue("fetch_inventory_on_login_check", mFetchInventoryOnLogin);
	childSetValue("enable_wind", mEnableLLWind);
	childSetValue("enable_clouds", mEnableClouds);
		childSetValue("enable_classic_clouds", mEnableClassicClouds);
	childSetValue("speed_rez_check", mSpeedRez);
	if (mSpeedRez)
	{
		childEnable("speed_rez_interval");
		childEnable("speed_rez_seconds");
	}
	else
	{
		childDisable("speed_rez_interval");
		childDisable("speed_rez_seconds");
	}
	//Command Line ------------------------------------------------------------------------

	//Privacy -----------------------------------------------------------------------------
	childSetValue("broadcast_viewer_effects", mBroadcastViewerEffects);
	childSetValue("disable_point_at_and_beams_check", mDisablePointAtAndBeam);
	childSetValue("private_look_at_check", mPrivateLookAt);
	childSetValue("revoke_perms_on_stand_up_check", mRevokePermsOnStandUp);
	
	childSetValue("enable_clouds", mEnableClouds);
	childSetValue("enable_classic_clouds", mEnableClassicClouds);

	gLLWindEnabled = mEnableLLWind;

	//Text Options ------------------------------------------------------------------------
    childSetValue("SpellDisplay", mSpellDisplay);

    childSetValue("Keywords_Alert", mKeywordsOn);
	childSetValue("Keywords_Entries", mKeywordsList);
	childSetValue("Keywords_LocalChat", mKeywordsInChat);
	childSetValue("Keywords_IM", mKeywordsInIM);
	childSetValue("Keywords_Highlight", mKeywordsChangeColor);
	childSetValue("Keywords_PlaySound", mKeywordsPlaySound);
	childSetValue("Keywords_SoundUUID", mKeywordsSound);

	LLColorSwatchCtrl* colorctrl = getChild<LLColorSwatchCtrl>("Keywords_Color");
	colorctrl->set(LLColor4(mKeywordsColor),TRUE);
*/}

void LLPrefsAscentSys::apply()
{
	std::string short_date, long_date, short_time, long_time, timestamp;
	
	//General ------------------------------------------------------------------------------
	gSavedSettings.setBOOL("DoubleClickTeleport",		childGetValue("double_click_teleport_check"));
		gSavedSettings.setBOOL("OptionRotateCamAfterLocalTP",	childGetValue("center_after_teleport_check"));
		gSavedSettings.setBOOL("OptionOffsetTPByAgentHeight",	childGetValue("offset_teleport_check"));
	gSavedSettings.setBOOL("PreviewAnimInWorld",		childGetValue("preview_anim_in_world_check"));
	gSavedSettings.setBOOL("SaveScriptsAsMono",			childGetValue("save_scripts_as_mono_check"));
	gSavedSettings.setBOOL("AscentAlwaysRezInGroup",	childGetValue("always_rez_in_group_check"));
	//Disable Teleport Progress
	//Disable Logout progress
	//always show Build
	gSavedSettings.setBOOL("AscentFlyAlwaysEnabled",	childGetValue("always_fly_check"));
	//Disable camera minimum distance
	gSavedSettings.setBOOL("AscentPowerfulWizard",		(childGetValue("power_user_check") && childGetValue("power_user_confirm_check")));
	if (gSavedSettings.getBOOL("AscentPowerfulWizard") && !mPowerUser)
	{
		LLVector3d lpos_global = gAgent.getPositionGlobal();
		gAudiop->triggerSound(LLUUID("58a38e89-44c6-c52b-deb8-9f1ddc527319"), gAgent.getID(), 1.0f, LLAudioEngine::AUDIO_TYPE_UI, lpos_global);
		LLChat chat;
		chat.mSourceType = CHAT_SOURCE_SYSTEM;
		chat.mText = llformat("You are bestowed with powers beyond mortal comprehension.\nUse your newfound abilities wisely.\nUnlocked:\n- Animation Priority up to 7 - Meant for animations that should override anything and everything at all times. DO NOT USE THIS FOR GENERAL ANIMATIONS.\n- Right click > Destroy objects - Permanently deletes an object immediately, when you don't feel like waiting for the server to respond.\n- Right Click > Explode objects - Turns and object physical, temporary, and delinks it.");
		LLFloaterChat::addChat(chat);
	}
		//
	//Chat/IM ------------------------------------------------------------------------------
	//Use Vertical IMs
	//Script count
	//Missing the echo/log option.
	gSavedSettings.setBOOL("PlayTypingSound",			childGetValue("play_typing_sound_check"));
	gSavedSettings.setBOOL("AscentHideTypingNotification",	childGetValue("hide_typing_check"));
	gSavedPerAccountSettings.setBOOL("AscentInstantMessageAnnounceIncoming",	childGetValue("AscentInstantMessageAnnounceIncoming").asBoolean());
	gSavedSettings.setBOOL("AscentAllowMUpose",			childGetValue("allow_mu_pose_check").asBoolean());
	gSavedSettings.setBOOL("AscentAutoCloseOOC",		childGetValue("close_ooc_check").asBoolean());
	//gSavedSettings.setU32("LinksForChattingObjects",	childGetValue("objects_link").   );
	
	LLComboBox* combo = getChild<LLComboBox>("time_format_combobox");
	if (combo) {
		mTimeFormat = combo->getCurrentIndex();
	}

	combo = getChild<LLComboBox>("date_format_combobox");
	if (combo)
	{
		mDateFormat = combo->getCurrentIndex();
	}

	if (mTimeFormat == 0)
	{
		short_time = "%H:%M";
		long_time  = "%H:%M:%S";
		timestamp  = " %H:%M:%S";
	}
	else
	{
		short_time = "%I:%M %p";
		long_time  = "%I:%M:%S %p";
		timestamp  = " %I:%M %p";
	}

	if (mDateFormat == 0)
	{
		short_date = "%Y-%m-%d";
		long_date  = "%A %d %B %Y";
		timestamp  = "%a %d %b %Y" + timestamp;
	}
	else if (mDateFormat == 1)
	{
		short_date = "%d/%m/%Y";
		long_date  = "%A %d %B %Y";
		timestamp  = "%a %d %b %Y" + timestamp;
	}
	else
	{
		short_date = "%m/%d/%Y";
		long_date  = "%A, %B %d %Y";
		timestamp  = "%a %b %d %Y" + timestamp;
	}

	gSavedSettings.setString("ShortDateFormat",			short_date);
	gSavedSettings.setString("LongDateFormat",			long_date);
	gSavedSettings.setString("ShortTimeFormat",			short_time);
	gSavedSettings.setString("LongTimeFormat",			long_time);
	gSavedSettings.setString("TimestampFormat",			timestamp);
	gSavedSettings.setBOOL("SecondsInChatAndIMs",		childGetValue("seconds_in_chat_and_ims_check").asBoolean());


	gSavedPerAccountSettings.setString("AscentInstantMessageResponse",			childGetValue("im_response").asString());
	gSavedPerAccountSettings.setBOOL("AscentInstantMessageResponseMuted",		childGetValue("AscentInstantMessageResponseMuted").asBoolean());
	gSavedPerAccountSettings.setBOOL("AscentInstantMessageResponseFriends",		childGetValue("AscentInstantMessageResponseFriends").asBoolean());
	gSavedPerAccountSettings.setBOOL("AscentInstantMessageResponseMuted",		childGetValue("AscentInstantMessageResponseMuted").asBoolean());
	gSavedPerAccountSettings.setBOOL("AscentInstantMessageResponseAnyone",		childGetValue("AscentInstantMessageResponseAnyone").asBoolean());
	gSavedPerAccountSettings.setBOOL("AscentInstantMessageShowResponded",		childGetValue("AscentInstantMessageShowResponded").asBoolean());
	gSavedPerAccountSettings.setBOOL("AscentInstantMessageShowOnTyping",		childGetValue("AscentInstantMessageShowOnTyping").asBoolean());
	gSavedPerAccountSettings.setBOOL("AscentInstantMessageResponseRepeat",		childGetValue("AscentInstantMessageResponseRepeat").asBoolean());
	gSavedPerAccountSettings.setBOOL("AscentInstantMessageResponseItem",		childGetValue("AscentInstantMessageResponseItem").asBoolean());
	gSavedPerAccountSettings.setBOOL("AscentInstantMessageAnnounceIncoming",	childGetValue("AscentInstantMessageAnnounceIncoming").asBoolean());

	//Performance ----------------------------------------------------------------------------
	gSavedSettings.setBOOL("FetchInventoryOnLogin",		childGetValue("fetch_inventory_on_login_check"));
	gSavedSettings.setBOOL("WindEnabled",				childGetValue("enable_wind"));
	gSavedSettings.setBOOL("CloudsEnabled",				childGetValue("enable_clouds"));
	gSavedSettings.setBOOL("SkyUseClassicClouds",		childGetValue("enable_classic_clouds"));
	gSavedSettings.setBOOL("SpeedRez",					childGetValue("speed_rez_check"));
	gSavedSettings.setU32("SpeedRezInterval",			childGetValue("speed_rez_interval").asReal());
	
	//Commandline ----------------------------------------------------------------------------
	//Missing "Use Command Line"
	gSavedSettings.setString("AscentCmdLinePos",		childGetValue("AscentCmdLinePos"));
	gSavedSettings.setString("AscentCmdLineGround",		childGetValue("AscentCmdLineGround"));
	gSavedSettings.setString("AscentCmdLineHeight",		childGetValue("AscentCmdLineHeight"));
	gSavedSettings.setString("AscentCmdLineTeleportHome",	childGetValue("AscentCmdLineTeleportHome"));
	gSavedSettings.setString("AscentCmdLineRezPlatform",	childGetValue("AscentCmdLineRezPlatform"));
	//Platform Size
	gSavedSettings.setString("AscentCmdLineCalc",		childGetValue("AscentCmdLineCalc"));
	gSavedSettings.setString("AscentCmdLineClearChat",	childGetValue("AscentCmdLineClearChat"));
		//-------------------------------------------------------------------------------------
	gSavedSettings.setString("AscentCmdLineDrawDistance",	childGetValue("AscentCmdLineDrawDistance"));
	gSavedSettings.setString("AscentCmdTeleportToCam",		childGetValue("AscentCmdTeleportToCam"));
	gSavedSettings.setString("AscentCmdLineKeyToName",		childGetValue("AscentCmdLineKeyToName"));
	gSavedSettings.setString("AscentCmdLineOfferTp",		childGetValue("AscentCmdLineOfferTp"));
	gSavedSettings.setString("AscentCmdLineMapTo",			childGetValue("AscentCmdLineMapTo"));
	gSavedSettings.setBOOL("AscentMapToKeepPos",			childGetValue("AscentMapToKeepPos"));
	gSavedSettings.setString("AscentCmdLineTP2",			childGetValue("AscentCmdLineTP2"));
	

	//Privacy --------------------------------------------------------------------------------
	gSavedSettings.setBOOL("BroadcastViewerEffects",	childGetValue("broadcast_viewer_effects"));
	gSavedSettings.setBOOL("DisablePointAtAndBeam",		childGetValue("disable_point_at_and_beams_check"));
	gSavedSettings.setBOOL("PrivateLookAt",				childGetValue("private_look_at_check"));
	LLHUDEffectLookAt::sDebugLookAt						= childGetValue("show_look_at_check");
	gSavedSettings.setBOOL("RevokePermsOnStandUp",		childGetValue("revoke_perms_on_stand_up_check"));
	gSavedSettings.setBOOL("DisableClickSit",			childGetValue("disable_click_sit_check"));


	//Text Options ---------------------------------------------------------------------------
    gSavedPerAccountSettings.setBOOL("KeywordsOn", childGetValue("Keywords_Alert"));
    gSavedPerAccountSettings.setString("KeywordsList", childGetValue("Keywords_Entries"));
    gSavedPerAccountSettings.setBOOL("KeywordsInChat", childGetValue("Keywords_LocalChat"));
    gSavedPerAccountSettings.setBOOL("KeywordsInIM", childGetValue("Keywords_IM"));
    gSavedPerAccountSettings.setBOOL("KeywordsChangeColor", childGetValue("Keywords_Highlight"));
    gSavedPerAccountSettings.setColor4("KeywordsColor", childGetValue("Keywords_Color"));
    gSavedPerAccountSettings.setBOOL("KeywordsPlaySound", childGetValue("Keywords_PlaySound"));
    gSavedPerAccountSettings.setString("KeywordsSound", childGetValue("Keywords_SoundUUID"));

	refreshValues();
	refresh();
}
