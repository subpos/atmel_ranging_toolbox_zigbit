var NAVTREE =
[
  [ "RTB", "index.html", [
    [ "Ranging Toolbox", "index.html", [
      [ "RTB Software Stack Architecture", "pgArchitecture.html", null ],
      [ "LIMITED LICENSE AGREEMENT", "pgLLA.html", null ],
      [ "References", "pgReferences.html", null ]
    ] ],
    [ "Modules", "modules.html", [
      [ "RTB API Functions", "group__apiRTB__API.html", null ]
    ] ],
    [ "Data Structures", "annotated.html", [
      [ "additional_result_fields_t", "unionadditional__result__fields__t.html", null ],
      [ "local_ranging_result_t", "structlocal__ranging__result__t.html", null ],
      [ "measurement_pair_tag", "structmeasurement__pair__tag.html", null ],
      [ "pmu_avg_data_t_tag", "structpmu__avg__data__t__tag.html", null ],
      [ "prov_antenna_div_results_tag", "structprov__antenna__div__results__tag.html", null ],
      [ "range_conf_result_t", "unionrange__conf__result__t.html", null ],
      [ "range_param_pmu_tag", "structrange__param__pmu__tag.html", null ],
      [ "range_param_tag", "structrange__param__tag.html", null ],
      [ "range_remote_answer_tag", "structrange__remote__answer__tag.html", null ],
      [ "range_status_pmu_tag", "structrange__status__pmu__tag.html", null ],
      [ "range_status_tag", "structrange__status__tag.html", null ],
      [ "refl_addr_tag", "structrefl__addr__tag.html", null ],
      [ "remote_ranging_result_t", "structremote__ranging__result__t.html", null ],
      [ "rtb_pmu_validity_ind_tag", "structrtb__pmu__validity__ind__tag.html", null ],
      [ "rtb_range_conf_tag", "structrtb__range__conf__tag.html", null ],
      [ "rtb_range_req_tag", "structrtb__range__req__tag.html", null ],
      [ "rtb_reset_conf_tag", "structrtb__reset__conf__tag.html", null ],
      [ "rtb_reset_req_tag", "structrtb__reset__req__tag.html", null ],
      [ "rtb_set_conf_tag", "structrtb__set__conf__tag.html", null ],
      [ "rtb_set_req_tag", "structrtb__set__req__tag.html", null ],
      [ "usr_rtb_pmu_validity_ind_tag", "structusr__rtb__pmu__validity__ind__tag.html", null ],
      [ "usr_rtb_range_conf_t", "structusr__rtb__range__conf__t.html", null ],
      [ "usr_rtb_reset_conf_tag", "structusr__rtb__reset__conf__tag.html", null ],
      [ "usr_rtb_set_conf_tag", "structusr__rtb__set__conf__tag.html", null ],
      [ "wpan_rtb_range_req_tag", "structwpan__rtb__range__req__tag.html", null ],
      [ "wpan_rtb_set_req_tag", "structwpan__rtb__set__req__tag.html", null ]
    ] ],
    [ "Data Structure Index", "classes.html", null ],
    [ "Data Fields", "functions.html", null ],
    [ "File List", "files.html", [
      [ "pal_config.h", "pal__config_8h.html", null ],
      [ "return_val.h", "return__val_8h.html", null ],
      [ "rtb.h", "rtb_8h.html", null ],
      [ "rtb_api.h", "rtb__api_8h.html", null ],
      [ "rtb_hw_233r_xmega.c", "rtb__hw__233r__xmega_8c.html", null ],
      [ "rtb_hw_233r_xmega.h", "rtb__hw__233r__xmega_8h.html", null ],
      [ "rtb_internal.h", "rtb__internal_8h.html", null ],
      [ "rtb_msg_const.h", "rtb__msg__const_8h.html", null ],
      [ "rtb_msg_types.h", "rtb__msg__types_8h.html", null ],
      [ "rtb_pmu.h", "rtb__pmu_8h.html", null ]
    ] ],
    [ "Globals", "globals.html", null ]
  ] ]
];

function createIndent(o,domNode,node,level)
{
  if (node.parentNode && node.parentNode.parentNode)
  {
    createIndent(o,domNode,node.parentNode,level+1);
  }
  var imgNode = document.createElement("img");
  if (level==0 && node.childrenData)
  {
    node.plus_img = imgNode;
    node.expandToggle = document.createElement("a");
    node.expandToggle.href = "javascript:void(0)";
    node.expandToggle.onclick = function() 
    {
      if (node.expanded) 
      {
        $(node.getChildrenUL()).slideUp("fast");
        if (node.isLast)
        {
          node.plus_img.src = node.relpath+"ftv2plastnode.png";
        }
        else
        {
          node.plus_img.src = node.relpath+"ftv2pnode.png";
        }
        node.expanded = false;
      } 
      else 
      {
        expandNode(o, node, false);
      }
    }
    node.expandToggle.appendChild(imgNode);
    domNode.appendChild(node.expandToggle);
  }
  else
  {
    domNode.appendChild(imgNode);
  }
  if (level==0)
  {
    if (node.isLast)
    {
      if (node.childrenData)
      {
        imgNode.src = node.relpath+"ftv2plastnode.png";
      }
      else
      {
        imgNode.src = node.relpath+"ftv2lastnode.png";
        domNode.appendChild(imgNode);
      }
    }
    else
    {
      if (node.childrenData)
      {
        imgNode.src = node.relpath+"ftv2pnode.png";
      }
      else
      {
        imgNode.src = node.relpath+"ftv2node.png";
        domNode.appendChild(imgNode);
      }
    }
  }
  else
  {
    if (node.isLast)
    {
      imgNode.src = node.relpath+"ftv2blank.png";
    }
    else
    {
      imgNode.src = node.relpath+"ftv2vertline.png";
    }
  }
  imgNode.border = "0";
}

function newNode(o, po, text, link, childrenData, lastNode)
{
  var node = new Object();
  node.children = Array();
  node.childrenData = childrenData;
  node.depth = po.depth + 1;
  node.relpath = po.relpath;
  node.isLast = lastNode;

  node.li = document.createElement("li");
  po.getChildrenUL().appendChild(node.li);
  node.parentNode = po;

  node.itemDiv = document.createElement("div");
  node.itemDiv.className = "item";

  node.labelSpan = document.createElement("span");
  node.labelSpan.className = "label";

  createIndent(o,node.itemDiv,node,0);
  node.itemDiv.appendChild(node.labelSpan);
  node.li.appendChild(node.itemDiv);

  var a = document.createElement("a");
  node.labelSpan.appendChild(a);
  node.label = document.createTextNode(text);
  a.appendChild(node.label);
  if (link) 
  {
    a.href = node.relpath+link;
  } 
  else 
  {
    if (childrenData != null) 
    {
      a.className = "nolink";
      a.href = "javascript:void(0)";
      a.onclick = node.expandToggle.onclick;
      node.expanded = false;
    }
  }

  node.childrenUL = null;
  node.getChildrenUL = function() 
  {
    if (!node.childrenUL) 
    {
      node.childrenUL = document.createElement("ul");
      node.childrenUL.className = "children_ul";
      node.childrenUL.style.display = "none";
      node.li.appendChild(node.childrenUL);
    }
    return node.childrenUL;
  };

  return node;
}

function showRoot()
{
  var headerHeight = $("#top").height();
  var footerHeight = $("#nav-path").height();
  var windowHeight = $(window).height() - headerHeight - footerHeight;
  navtree.scrollTo('#selected',0,{offset:-windowHeight/2});
}

function expandNode(o, node, imm)
{
  if (node.childrenData && !node.expanded) 
  {
    if (!node.childrenVisited) 
    {
      getNode(o, node);
    }
    if (imm)
    {
      $(node.getChildrenUL()).show();
    } 
    else 
    {
      $(node.getChildrenUL()).slideDown("fast",showRoot);
    }
    if (node.isLast)
    {
      node.plus_img.src = node.relpath+"ftv2mlastnode.png";
    }
    else
    {
      node.plus_img.src = node.relpath+"ftv2mnode.png";
    }
    node.expanded = true;
  }
}

function getNode(o, po)
{
  po.childrenVisited = true;
  var l = po.childrenData.length-1;
  for (var i in po.childrenData) 
  {
    var nodeData = po.childrenData[i];
    po.children[i] = newNode(o, po, nodeData[0], nodeData[1], nodeData[2],
        i==l);
  }
}

function findNavTreePage(url, data)
{
  var nodes = data;
  var result = null;
  for (var i in nodes) 
  {
    var d = nodes[i];
    if (d[1] == url) 
    {
      return new Array(i);
    }
    else if (d[2] != null) // array of children
    {
      result = findNavTreePage(url, d[2]);
      if (result != null) 
      {
        return (new Array(i).concat(result));
      }
    }
  }
  return null;
}

function initNavTree(toroot,relpath)
{
  var o = new Object();
  o.toroot = toroot;
  o.node = new Object();
  o.node.li = document.getElementById("nav-tree-contents");
  o.node.childrenData = NAVTREE;
  o.node.children = new Array();
  o.node.childrenUL = document.createElement("ul");
  o.node.getChildrenUL = function() { return o.node.childrenUL; };
  o.node.li.appendChild(o.node.childrenUL);
  o.node.depth = 0;
  o.node.relpath = relpath;

  getNode(o, o.node);

  o.breadcrumbs = findNavTreePage(toroot, NAVTREE);
  if (o.breadcrumbs == null)
  {
    o.breadcrumbs = findNavTreePage("index.html",NAVTREE);
  }
  if (o.breadcrumbs != null && o.breadcrumbs.length>0)
  {
    var p = o.node;
    for (var i in o.breadcrumbs) 
    {
      var j = o.breadcrumbs[i];
      p = p.children[j];
      expandNode(o,p,true);
    }
    p.itemDiv.className = p.itemDiv.className + " selected";
    p.itemDiv.id = "selected";
    $(window).load(showRoot);
  }
}

