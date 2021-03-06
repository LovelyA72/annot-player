-- acfun.lua
-- 2011/11/6

require "lalib"

-- jichi 2/4/2012: replace with permanent name
--acfun_xml_servername = '124.228.254.234';--'www.sjfan.com.cn';
--acfun_xml_servername = "www.sjfan.com.cn";

acfun_xml_servername = 'www.acfun.tv'; --'124.228.254.234';--'www.sjfan.com.cn';
acfun_comment_servername = 'comment.acfun.tv';--'122.224.11.162';--

--[[parse single acfun url]]
function getTaskAttribute_acfun ( str_url, str_tmpfile ,str_servername, pDlg, bSubOnly)
  clib.debug("acfun: enter")

  if pDlg~=nil then
    sShowMessage(pDlg, '开始解析..');
  end
  local int_acfpv = getACFPV ( str_url, str_servername );

  -------[[read flv id start]]

  local re = dlFile(str_tmpfile, str_url);
  if re~=0
  then
    if pDlg~=nil then
      sShowMessage(pDlg, '读取原始页面错误。');
    end
    clib.debug("acfun: abort: download failed")
    return;
  else
    if pDlg~=nil then
      sShowMessage(pDlg, '已读取原始页面，正在分析...');
    end
  end

  --dbgMessage("dl ok");
  local file = io.open(str_tmpfile, "r");
  if file==nil
  then
    if pDlg~=nil then
      sShowMessage(pDlg, '读取原始页面错误。');
    end
    clib.debug("acfun: abort: open file failed")
    return;
  end

  local int_foreignlinksite = fls["realurl"];

  local str_id = "";

  local str_aid = "";

  local str_subid = str_id;

  local str_descriptor = "";

  local str_tmp_vd = "";

  local str_title = "";

  --isFramework?
  local isFramework = 0;
  local str_line = readUntilFromUTF8(file, "<html");
  local str_meta_line = readIntoUntilFromUTF8(file, str_line, "<!--title-->");
  --dbgMessage(str_meta_line);
  --dbgMessage(string.find(str_meta_line, "<!--meta-->",1 ,true));
  if string.find(str_meta_line, "<!--meta-->", 1, true)~=nil then
   clib.debug("acfun: read meta title")
    --is Framework
    isFramework = 1;
    --dbgMessage("framework");

    --readin descriptor
    str_line = readUntilFromUTF8(file, "<title>");
    --dbgMessage(str_line);
    local str_title_line = readIntoUntilFromUTF8(file, str_line, "</title>");
    str_title = getMedText(str_title_line, "<title>", "</title>");

    --dbgMessage(str_title);

    str_line = readUntilFromUTF8(file, "system.aid");
    local transid = getMedText(str_line, "system.aid = ", ";");
    --dbgMessage(transid);

    local tbl_id_titles = getAcVideo_Vid_Cid_Titles(transid, str_tmpfile .. ".tmpacapi", pDlg);

    --dbgMessage(tbl_id_titles["1"]["desp"]);
    --dbgMessage(str_url);
    local _, _, str_vindex = string.find(str_url, "ep=(%d+)");

    if str_vindex==nil then
      str_vindex = "1";
    end
    --dbgMessage(str_vindex);

    str_id = tbl_id_titles[str_vindex]["vid"];
    str_aid = tbl_id_titles[str_vindex]["cid"];
    str_subid = tbl_id_titles[str_vindex]["vid"];
    int_foreignlinksite = fls["iqiyi"];
    str_tmp_vd = tbl_id_titles[str_vindex]["desp"];

  else
   clib.debug("acfun: read real title")

    --readin descriptor
    str_line = readUntilFromUTF8(file, "<title>");
    --dbgMessage(str_line);
    local str_title_line = readIntoUntilFromUTF8(file, str_line, "</title>");
    str_title = getMedText(str_title_line, "<title>", "</title>");

    --dbgMessage(str_title);
    --readin vice descriptor
    --readUntil(file, "主页</a>");
    --readUntilFromUTF8(file, "</div><!--Tool -->");
    --readUntilFromUTF8(file, "</div><!--Title -->");
    --readUntilFromUTF8(file ,"<div id=\"area-pager\" class=\"area-pager\">");
    readUntilFromUTF8(file , "<div id=\"area-part-view\"");
    str_line = "";

    --while str_line~=nil and string.find(str_line, "</tr>")==nil
    while str_line~=nil and string.find(str_line, "</div>")==nil
    --while str_line~=nil and string.find(str_line, "结束")==nil
    do
      str_line = file:read("*l");
      str_line = utf8_to_lua(str_line);
      --dbgMessage(str_line);
      --if str_line~=nil and string.find(str_line, "<option value='")~=nil
      if str_line~=nil and string.find(str_line, "<a class=\"")~=nil
      then
        --dbgMessage("pager article");
        --if str_tmp_vd=="" or string.find(str_line, "selected>")~=nil
        --if str_tmp_vd=="" or string.find(str_line, "pager active")~=nil
        if str_tmp_vd=="" or string.find(str_line, "success active")~=nil
        then
          --dbgMessage("pager active");
          --str_tmp_vd = getMedText(str_line, ">", "</option>");
          str_tmp_vd = getMedText(str_line, "/i>", "</a>");

          local str_acinternalID = getMedText(str_line, "data-vid=\"", "\"");

          --dbgMessage(str_acinternalID);

          int_foreignlinksite, str_id, str_subid = getAcVideo_CommentID(str_acinternalID, str_tmpfile..".tmpac", pDlg);

        end
      end
    end

  end
  --save descriptor

  if str_tmp_vd ~= ""
  then
    str_descriptor = str_title.."-"..str_tmp_vd;
  else
    str_descriptor = str_title;
  end

  --dbgMessage(str_title);
  --dbgMessage(str_tmp_vd);
  --dbgMessage(str_descriptor);

--~   --find embed flash
--~   --str_line = readUntilFromUTF8(file, "<embed ");
--~   str_line = readUntilFromUTF8(file, "<div id=\"area-player\"");
--~   local str_embed = readIntoUntilFromUTF8(file, str_line, "<script>");--edit 20121127 </div>");--"</td>");--"</embed>");
--~   print(str_embed);
--~   --dbgMessage(str_embed);
--~   if str_embed==nil then
--~     if pDlg~=nil then
--~       sShowMessage(pDlg, "没有找到嵌入的flash播放器");
--~     end
--~     io.close(file);
--~     return;
--~   end

--~   local b_isArtemis = 0;
--~   if string.find(str_embed, "Artemis",1,true)~=nil or string.find(str_embed, "[Video]",1,true)~=nil  or string.find(str_embed, "[video]",1,true)~=nil then
--~     b_isArtemis = 1;
--~     --dbgMessage("artemis");
--~   end

--~   local int_foreignlinksite = fls["realurl"];
--~   local str_id = "";
--~   local str_subid = str_id;
--~   if b_isArtemis==0 then
--~     --dbgMessage("old");
--~     --read foreign file
--~     local str_notsinaurl = "";
--~     if string.find(str_embed, "flashvars=\"file=", 1, true)~=nil
--~     then
--~       str_notsinaurl = getMedText2end(str_embed, "flashvars=\"file=", "\"", "&amp;");
--~     elseif string.find(str_embed, "playerf.swf?file=")~=nil
--~     then
--~       str_notsinaurl = getMedText2end(str_embed, "playerf.swf?file=", "\"", "&amp;");
--~     elseif string.find(str_embed, "file=")~=nil
--~     then
--~       str_notsinaurl = getMedText2end(str_embed, "file=", "\"", "&amp;");
--~     end

--~     --certain foreign sitelink
--~     if str_notsinaurl=="" then
--~       if string.find(str_embed, "type2=qq", 1,true)~=nil then
--~         int_foreignlinksite = fls["qq"];
--~       elseif string.find(str_embed, "type2=youku", 1,true)~=nil then
--~         int_foreignlinksite = fls["youku"];
--~       elseif string.find(str_embed, "type2=tudou", 1,true)~=nil then
--~         int_foreignlinksite = fls["tudou"];
--~       else
--~         int_foreignlinksite = fls["sina"];
--~       end
--~     end

--~     --certain acfpv
--~     if string.find(str_embed, "flvplayer/acplayer.swf")~=nil or string.find(str_embed, "flvplayer/acplayert.swf")~=nil
--~     then
--~       int_acfpv = 0; -- ACFPV_ORI
--~     end

--~     --read id

--~     if string.find(str_embed, "flashvars=\"id=")~=nil
--~     then
--~       str_id = getMedText2end(str_embed, "flashvars=\"id=", "\"", "&amp;");
--~     elseif string.find(str_embed, "flashvars=\"avid=")~=nil
--~     then
--~       str_id = getMedText2end(str_embed, "flashvars=\"avid=", "\"", "&amp;");
--~     elseif string.find(str_embed, "?id=")~=nil
--~     then
--~       str_id = getMedText2end(str_embed, "?id=", "\"", "&amp;");
--~     elseif string.find(str_embed, "id=")~=nil
--~     then
--~       str_embed_tmp = getAfterText(str_embed, "flashvars=");
--~       if str_embed_tmp==nil
--~       then
--~         str_embed_tmp = getAfterText(str_embed, "src=");
--~       end
--~       str_id = getMedText2end(str_embed_tmp, "id=", "\"", "&amp;");
--~     --elseif string.find(str_embed, "[Video]")~=nil
--~     --then
--~     --  str_id = getMedText(str_embed, "[Video]", "[/Video]");
--~     --  int_foreignlinksite,str_id, str_subid, = getAcVideo_CommentID(str_id,
--~     --  str_subid="";--not com
--~     --  str_id="";--
--~     end

--~     --dbgMessage(str_id);
--~     --if there is a seperate subid
--~     if string.find(str_embed, "mid=")~= nil then
--~       str_subid = getMedText2end(str_embed, "mid=", "\"", "&amp;");
--~     elseif string.find(str_embed, "cid=")~= nil then
--~       str_subid = getMedText2end(str_embed, "cid=", "\"", "&amp;");
--~     end
--~   elseif b_isArtemis==1 then
--~     int_acfpv = 1; -- ACFPV_NEW
--~     local str_acinternalID = getMedText(str_embed, "{'id':'", "','system'");
--~     if str_acinternalID == nil then
--~       str_acinternalID = getMedText(str_embed, "[Video]", "[/Video]");
--~     end
--~     if str_acinternalID == nil then
--~       str_acinternalID = getMedText(str_embed, "[video]", "[/video]");
--~     end
--~     --dbgMessage(str_acinternalID);

--~     int_foreignlinksite, str_id, str_subid = getAcVideo_CommentID(str_acinternalID, str_tmpfile..".tmpac", pDlg);

--~     --dbgMessage(str_id);
--~     --dbgMessage(str_subid);
--~   end

  --dbgMessage(str_id);

  --read id ok.close file
  io.close(file);

  if str_id == ""
  then
    if pDlg~=nil then
      sShowMessage(pDlg, '解析FLV ID错误。');
    end
    clib.debug("acfun: abort: str id is empty")
    return;
  end
  ------------------------------------------------------------[[read flv id end]]


  --dbgMessage(str_id);

  if str_subid=="" then
    str_subid = str_id;
  end
  --define subxmlurl
  local str_subxmlurl = "";
  local str_subxmlurl_lock = "";
  local str_subxmlurl_super = "";
  if int_acfpv==1 --ACFPV_NEW
  then
    str_subxmlurl = "http://"..acfun_xml_servername.."/newflvplayer/xmldata/"..str_subid.."/comment_on.xml?r=1";
    str_subxmlurl_lock = "http://"..acfun_xml_servername.."/newflvplayer/xmldata/"..str_subid.."/comment_lock.xml?r=1";
    str_subxmlurl_super = "http://"..acfun_xml_servername.."/newflvplayer/xmldata/"..str_subid.."/comment_super.xml";
    str_subxmlurl_20111106 = "http://"..acfun_comment_servername.."/".. str_subid ..".json?clientID=0.9264421034604311";
    str_subxmlurl_lock_20111106 = "http://"..acfun_comment_servername.."/".. str_subid .."_lock.json?clientID=0.4721592585556209";
    --str_subxmlurl_20111106 = "http://"..acfun_comment_servername.."/".. str_subid ..".json";
    --str_subxmlurl_lock_20111106 = "http://"..acfun_comment_servername.."/".. str_subid .."_lock.json";
  else --ACFPV_ORI
    str_subxmlurl = "http://"..acfun_xml_servername.."/flvplayer/xmldata/"..str_subid.."/comment_on.xml?a=1";
  end

  -- jichi 2/4/2012: Enforce JSON format
  str_subxmlurl = "http://comment.acfun.tv/".. str_subid ..".json?clientID=0.9264421034604311";

  --realurls
  local int_realurlnum = 0;
  local tbl_realurls = {};
  --if str_notsinaurl=="" -- is sina flv
  if int_foreignlinksite == fls["sina"]
  then
    --fetch dynamic url
    int_realurlnum, tbl_readurls = getRealUrls(str_id, str_tmpfile, pDlg);
  elseif int_foreignlinksite == fls["qq"]
  then
    int_realurlnum, tbl_readurls = getRealUrls_QQ(str_id, str_tmpfile, pDlg);
  elseif int_foreignlinksite == fls["youku"]
  then
    int_realurlnum, tbl_readurls = getRealUrls_youku(str_id, str_tmpfile, pDlg);
  elseif int_foreignlinksite == fls["tudou"]
  then
    int_realurlnum, tbl_readurls = getRealUrls_tudou(str_id, str_tmpfile, pdlg);
  elseif int_foreignlinksite == fls["pps"]
  then
    int_realurlnum, tbl_readurls = getRealUrls_pps(str_id, str_tmpfile, pdlg);
  elseif int_foreignlinksite == fls["iqiyi"]
  then
    int_realurlnum, tbl_readurls = getRealUrls_iqiyi(str_id, str_tmpfile, pDlg);
  else
    int_realurlnum = 1;
    tbl_readurls = {};
    tbl_readurls[string.format("%d",0)] = str_notsinaurl;
  end

  if pDlg~=nil then
    sShowMessage(pDlg, '完成解析..');
  end

  local tbl_subxmlurls={};
  tbl_subxmlurls["0"] = str_subxmlurl;
  if str_subxmlurl_lock ~= "" then
    tbl_subxmlurls["1"] = str_subxmlurl_lock;
    tbl_subxmlurls["2"] = str_subxmlurl_super;
    tbl_subxmlurls["3"] = str_subxmlurl_20111106;
    tbl_subxmlurls["4"] = str_subxmlurl_lock_20111106;
  end

  local _, _, str_acfid = string.find(str_url, "/([%d_]+).html");
  local str_acfid_subfix = nil;
  if str_acfid == nil
  then
    _,_,str_acfid = string.find(str_url, "/[a]?[c]?([%d_]+)/?");
    _,_, str_acfid_subfix = string.find(str_url, "/index([%d_]*).html");
    if str_acfid_subfix ~= nil then
      str_acfid = str_acfid .. str_acfid_subfix;
    end
  end
  --for http://acfun.tv/plus/view.php?aid=xxxxxxx
  if str_acfid == nil
  then
    _,_,str_acfid = string.find(str_url, "/view.php%?aid=([%d_]+)");
    _,_,str_acfid_subfix = string.find(str_url, "pageno=([%d]+)");
    if str_acfid_subfix ~= nil then
      str_acfid = str_acfid .. str_acfid_subfix;
    end
  end

  --for http://hengyang.acfun.tv/sp/aqgy/?ep=2
  if str_acfid == nil
  then
    _, _, str_acfid_subfix = string.find(str_url, "ep=(%d+)");
    if str_acfid_subfix ~= nil then
      str_acfid = str_aid .. str_acfid_subfix;
    else
      str_acfid = str_aid;
    end
  end
  local tbl_ta = {};
  tbl_ta["acfpv"] = int_acfpv;
  tbl_ta["descriptor"] = "ac"..str_acfid.." - "..str_descriptor;
  --tbl_ta["subxmlurl"] = str_subxmlurl;
  tbl_ta["subxmlurl"] = tbl_subxmlurls;
  tbl_ta["realurlnum"] = int_realurlnum;
  tbl_ta["realurls"] = tbl_readurls;
  tbl_ta["oriurl"] = str_url;

  local tbl_resig = {};
  tbl_resig[string.format("%d",0)] = tbl_ta;

  clib.debug("acfun: leave: ok")
  return tbl_resig;
end

--[[parse every video in acfun url]]
function getTaskAttributeBatch_acfun ( str_url, str_tmpfile ,str_servername, pDlg)
  if pDlg~=nil then
    sShowMessage(pDlg, '开始解析..');
  end

  local re = dlFile(str_tmpfile, str_url);
  if re~=0
  then
    if pDlg~=nil then
      sShowMessage(pDlg, '读取原始页面错误。');
    end
    return;
  else
    if pDlg~=nil then
      sShowMessage(pDlg, '已读取原始页面，正在分析...');
    end
  end

  local file = io.open(str_tmpfile, "r");
  if file==nil
  then
    if pDlg~=nil then
      sShowMessage(pDlg, '读取原始页面错误。');
    end
    return;
  end

  --readin descriptor
  --local str_line = readUntil(file, "<title>");
  --local str_title_line = readIntoUntil(file, str_line, "</title>");
  local str_line = readUntilFromUTF8(file, "<title>");
  local str_title_line = readIntoUntilFromUTF8(file, str_line, "</title>");
  local str_title = getMedText(str_title_line, "<title>", "</title>");

  --readin vice descriptor
  --readUntil(file, "主页</a>");
    readUntilFromUTF8(file, "</div><!--Tool -->");

  str_line = "";
  local tbl_descriptors = {};
  local tbl_shorturls = {};
  local index = 0;
  --while str_line~=nil and string.find(str_line, "</tr>")==nil
    while str_line~=nil and string.find(str_line, "</div>")==nil--"</tr>")==nil
  do
    str_line = file:read("*l");
    --if str_line~=nil and string.find(str_line, "<option value='")~=nil
        str_line = utf8_to_lua(str_line);
    if str_line~=nil and string.find(str_line, "<a class")~=nil
    then
      --local str_tmp_vd = getMedText(str_line, ">", "</option>");
      --local str_tmp_url = getMedText(str_line, "<option value='", "'");
            local str_tmp_vd = getMedText(str_line, "\">", "</a>");
      local str_tmp_url = getMedText(str_line, "href=\"", "\">");
      local str_index = string.format("%d", index);
      tbl_descriptors[str_index] = --[[str_title.."-"..]]str_tmp_vd;
      tbl_shorturls[str_index] = str_tmp_url;
      index = index+1;
    end
  end

  --readin pairs ok. close file
  io.close(file);

  if index==0 then
    local str_index = string.format("%d", index);
    tbl_descriptors[str_index] = str_title;
    local bg_t,ed_t = string.find(string.reverse(str_url),"/",1,true);
    ed_t = string.len(str_url)+1-ed_t;
    tbl_shorturls[str_index] = string.sub(str_url, ed_t+1) ;
    index = index+1;
  end

  --dbgMessage(string.format("%d", index));

  --------[[parse every url in pairs]]

  --local bg,ed = string.find(string.reverse(str_url),"/",1,true);
  --ed = string.len(str_url)+1-ed;
  --local urlprefix = string.sub(str_url, 1, ed);
  local urlprefix = "http://www.acfun.tv/v/"

  local tbl_re = {};
  local index2= 0;
  for ti = 0, index-1, 1 do
    local str_index = string.format("%d", ti);
    sShowMessage(pDlg, string.format("正在解析地址(%d/%d)\"%s\",请等待..",ti+1,index,tbl_shorturls[str_index]));
    for tj = 0, 5, 1 do
      local str_son_url = urlprefix..tbl_shorturls[str_index];
      --dbgMessage(str_son_url);
      local tbl_sig = getTaskAttribute_acfun(str_son_url, str_tmpfile, str_servername, nil);
      if tbl_sig~=nil then
        --dbgMessage("has video");
        --dbgMessage(tbl_sig["0"]["descriptor"]);
        --dbgMessage(tbl_descriptors[str_index]);
        --tbl_sig["0"]["descriptor"] = tbl_sig["0"]["descriptor"].." - "..tbl_descriptors[str_index];
        local str_index2 = string.format("%d",index2);
        tbl_re[str_index2] = deepcopy(tbl_sig)["0"];--dumpmytable(tbl_sig["0"]);--
        index2 = index2+1;
        break;
      else
        --dbgMessage("parse error");
      end
    end

  end

  sShowMessage(pDlg, string.format("解析完毕, 共有%d个视频",index2));

  return tbl_re;


end
