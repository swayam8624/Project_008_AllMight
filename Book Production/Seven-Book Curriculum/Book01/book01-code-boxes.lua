-- Project AllMight, Book One — reader-only Pandoc LaTeX presentation.
-- Never alter source Markdown or the contents of fenced programs.
-- This filter runs only on *generated* TeX output, after source-preservation
-- audits have already checked every chapter and teaching addition.
--
-- Unlike Pandoc's --listings switch, this preserves Pandoc's normal inline
-- \texttt output (including nested C++ template syntax) while giving fenced
-- code blocks language-aware, breakable tcolorbox/listings environments.
local mapping = {
  cpp = {"C++", "C++23"}, ["c++"] = {"C++", "C++23"},
  cxx = {"C++", "C++23"}, cc = {"C++", "C++23"},
  hpp = {"C++", "C++23"}, h = {"C++", "C++23"},
  c = {"C", "C"}, python = {"Python", "Python"},
  py = {"Python", "Python"}, javascript = {"JavaScript", "JavaScript"},
  js = {"JavaScript", "JavaScript"},
  bash = {"BOneShell", "Shell"}, sh = {"BOneShell", "Shell"},
  shell = {"BOneShell", "Shell"}, zsh = {"BOneShell", "Shell"},
  console = {"BOneShell", "Shell"},
  cmake = {"BOneCMake", "CMake"}, json = {"BOneJSON", "JSON"},
  glsl = {"BOneShader", "GLSL"}, hlsl = {"BOneShader", "HLSL"},
  metal = {"BOneShader", "Metal"}, msl = {"BOneShader", "Metal"},
  wgsl = {"BOneShader", "WGSL"},
}
local function map_language(block)
  for _, class in ipairs(block.classes) do
    local known = mapping[string.lower(class)]
    if known then return known end
  end
  local field = block.attributes and block.attributes.language or nil
  if field then
    local known = mapping[string.lower(field)]
    if known then return known end
  end
  return nil
end

function CodeBlock(block)
  if FORMAT ~= "latex" then return nil end
  -- A literal closing environment would terminate a verbatim listing early.
  -- Refuse the production transformation rather than silently corrupt it.
  if string.find(block.text, "\\end{BookCode}", 1, true) then
    error("Book One source listing contains the reserved \\end{BookCode} delimiter")
  end
  local language = map_language(block)
  local begin = "\\begin{BookCode}"
  if language then
    begin = begin .. "[language={" .. language[1] .. "}]"
  end
  return pandoc.RawBlock(
    "latex", begin .. "\n" .. block.text .. "\n\\end{BookCode}"
  )
end

local note_labels = {
  ["definition"] = "Definition",
  ["key idea"] = "Key idea",
  ["important"] = "Important",
  ["remember"] = "Remember",
  ["warning"] = "Warning",
  ["exercise"] = "Exercise",
  ["checkpoint"] = "Checkpoint",
}
function BlockQuote(block)
  if FORMAT ~= "latex" and not FORMAT:match("^epub") then return nil end
  local first = block.content[1]
  local plain = first and pandoc.utils.stringify(first) or ""
  local category = string.lower(plain):match("^%s*([%a%s]+)%s*:")
  local label = category and note_labels[(category:gsub("%s+$", ""))] or nil
  if FORMAT:match("^epub") then
    if not label then return nil end
    local role = (label == "Exercise" or label == "Checkpoint")
      and "book-practice" or "book-learning-note"
    local heading = pandoc.Para({pandoc.Strong({pandoc.Str(label)})})
    local contents = {heading}
    for _, item in ipairs(block.content) do contents[#contents + 1] = item end
    return pandoc.Div(contents, pandoc.Attr("", {role}))
  end
  local box = (label == "Exercise" or label == "Checkpoint")
      and "BookPractice" or "BookQuote"
  -- Explicitly labelled concepts become learning callouts. Ordinary
  -- quotations get a less emphatic quote panel, not a misleading Note label.
  if label and box ~= "BookPractice" then box = "BookLearningNote" end
  local opening = "\\begin{" .. box .. "}"
  if label and box ~= "BookQuote" then opening = opening .. "[" .. label .. "]" end
  local transformed = {pandoc.RawBlock("latex", opening)}
  for _, item in ipairs(block.content) do
    transformed[#transformed + 1] = item
  end
  transformed[#transformed + 1] = pandoc.RawBlock("latex", "\\end{" .. box .. "}")
  return transformed
end

function Para(block)
  if FORMAT ~= "latex" and not FORMAT:match("^epub") then return nil end
  local plain = pandoc.utils.stringify(block)
  local label = nil
  if plain:match("^%s*By the end:") then
    label = "Learning objective"
  elseif plain:match("^%s*Recall question:") then
    label = "Check your understanding"
  end
  if not label then return nil end
  if FORMAT:match("^epub") then
    return pandoc.Div({
      pandoc.Para({pandoc.Strong({pandoc.Str(label)})}),
      block,
    }, pandoc.Attr("", {"book-learning-note"}))
  end
  return {
    pandoc.RawBlock("latex", "\\begin{BookLearningNote}[" .. label .. "]"),
    block,
    pandoc.RawBlock("latex", "\\end{BookLearningNote}"),
  }
end
