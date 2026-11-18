-- Preserve explicit source role headings and mark only their lead paragraph.
-- The Div classes remain available to semantic formats such as EPUB; the
-- LaTeX writer maps them to the matching non-numbered style environments.

local roles = {
  { name = "definition", patterns = { "^definition%f[%W]", "^definitions%f[%W]" } },
  { name = "theorem", patterns = { "^theorem%f[%W]", "^lemma%f[%W]", "^proposition%f[%W]" } },
  { name = "example", patterns = { "^worked example%f[%W]", "^example%f[%W]" } },
  { name = "exercise", patterns = { "^exercise%f[%W]", "^practice%f[%W]" } },
  { name = "activerecall", patterns = { "^active recall%f[%W]" } },
}

local function role_for(header)
  local title = pandoc.utils.stringify(header.content):lower()
  title = title:gsub("^%s+", ""):gsub("%s+$", "")
  title = title:gsub("^%d+[%d%.]*%s+", "")
  for _, role in ipairs(roles) do
    for _, pattern in ipairs(role.patterns) do
      if title:match(pattern) then return role.name end
    end
  end
end

local function annotate(blocks)
  local out, index = {}, 1
  while index <= #blocks do
    local block = blocks[index]
    local role = block.t == "Header" and role_for(block) or nil
    local lead = blocks[index + 1]
    local target = nil
    if FORMAT:match("latex") and block.t == "Header" and block.identifier and block.identifier ~= "" then
      target = pandoc.RawBlock("latex", "\\hypertarget{" .. block.identifier .. "}{}")
    end
    if role and lead and (lead.t == "Para" or lead.t == "Plain") then
      if FORMAT:match("latex") then
        local env = "allmightrole" .. role
        out[#out + 1] = pandoc.RawBlock("latex", "\\begin{" .. env .. "}")
        out[#out + 1] = block
        if target then out[#out + 1] = target end
        out[#out + 1] = lead
        out[#out + 1] = pandoc.RawBlock("latex", "\\end{" .. env .. "}")
      else
        out[#out + 1] = pandoc.Div({ block, lead }, pandoc.Attr("", {
          "allmight-source-role", "allmight-role-" .. role
        }, { ["data-role"] = role }))
      end
      index = index + 2
    else
      out[#out + 1] = block
      if target then out[#out + 1] = target end
      index = index + 1
    end
  end
  return out
end

function Pandoc(doc)
  doc.blocks = annotate(doc.blocks)
  return doc
end

-- Hyperref interprets `#nameddest=name` in an external PDF URL as the literal
-- remote destination name `nameddest=name`. For LaTeX output, emit `#name` so
-- the PDF GoToR action targets the sibling file's actual named destination.
-- Keep the semantic AST target unchanged for HTML/EPUB and other formats.
function Link(link)
  if not FORMAT:match("latex") then return nil end
  local path, destination = link.target:match("^(.*%.pdf)#nameddest=([^#]+)$")
  if not path then return nil end
  link.target = path .. "#" .. destination
  return link
end

local escaped_code_characters = {}

local function escaped_code_character(character)
  local cached = escaped_code_characters[character]
  if cached then return cached end
  local native = pandoc.write(pandoc.Pandoc({ pandoc.Para({ pandoc.Code(character) }) }), "latex")
  local escaped = native:match("^\\texttt{(.*)}%s*$")
  if not escaped then error("Could not escape inline Code character: " .. native) end
  escaped_code_characters[character] = escaped
  return escaped
end

function Code(code)
  if not FORMAT:match("latex") then return nil end
  local native = pandoc.write(pandoc.Pandoc({ pandoc.Para({ code }) }), "latex")
  local escaped = native:match("^\\texttt{(.*)}%s*$")
  if not escaped then error("Could not extract Pandoc's escaped inline Code body: " .. native) end
  -- Do not let seqsplit see TeX control sequences such as \^{} as ordinary
  -- characters. Each source character is a group, so it can add breakpoints
  -- only between complete escaped glyphs. Keep Pandoc's native escaped text
  -- as the bookmark fallback.
  local display = code.text:gsub("\t", "    "):gsub("\r", ""):gsub("\n", "")
  local atoms = {}
  for _, cp in utf8.codes(display) do
    local character = utf8.char(cp)
    if character == " " then
      atoms[#atoms + 1] = "{\\ }"
    else
      atoms[#atoms + 1] = "{" .. escaped_code_character(character) .. "}"
    end
  end
  return {
    pandoc.RawInline("latex", "\\BookInlineCode{" .. table.concat(atoms) .. "}{"),
    pandoc.RawInline("latex", escaped),
    pandoc.RawInline("latex", "}")
  }
end

function Div(div)
  if not FORMAT:match("latex") then return nil end
  local role
  for _, class in ipairs(div.classes) do
    local candidate = class:match("^allmight%-role%-(.+)$")
    if candidate then role = candidate; break end
  end
  if not role then return nil end
  local env = "allmightrole" .. role
  local blocks = { pandoc.RawBlock("latex", "\\begin{" .. env .. "}") }
  for _, block in ipairs(div.content) do blocks[#blocks + 1] = block end
  blocks[#blocks + 1] = pandoc.RawBlock("latex", "\\end{" .. env .. "}")
  return blocks
end
