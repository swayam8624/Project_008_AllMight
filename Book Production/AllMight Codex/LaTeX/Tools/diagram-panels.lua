-- Render approved source-aware print diagram variants with the wider panel cap.
-- The image remains a normal Pandoc image for non-LaTeX writers; only the
-- print LaTeX conversion maps the explicit class to the book's panel macro.

local function has_class(image, wanted)
  for _, class in ipairs(image.classes) do
    if class == wanted then return true end
  end
  return false
end

local function path_caption(uri)
  local filename = uri:gsub("\\", "/"):match("([^/]+)$")
  if not filename or not filename:match("%.[%a%d]+$") then return nil end
  local stem = filename:gsub("%.[^.]+$", ""):gsub("[_%-]+", " ")
  if stem == "" then return nil end
  return stem:sub(1, 1):upper() .. stem:sub(2)
end

local function latex_escape(value)
  local replacements = {
    ["\\"] = "\\textbackslash{}", ["{"] = "\\{", ["}"] = "\\}",
    ["$"] = "\\$", ["&"] = "\\&", ["#"] = "\\#", ["%"] = "\\%",
    ["_"] = "\\_", ["^"] = "\\textasciicircum{}", ["~"] = "\\textasciitilde{}"
  }
  return value:gsub("[\\{}$&#%%_^~]", replacements)
end

function Image(image)
  if not FORMAT:match("latex") then return nil end
  if not has_class(image, "allmight-auto-path-caption") then return nil end
  local uri = image.attr.attributes["data-auto-path-caption-uri"]
  if not uri or pandoc.utils.stringify(image.caption) ~= uri then return nil end
  local human = path_caption(uri)
  if not human then return nil end
  image.caption = pandoc.Inlines({ pandoc.Str(human) })
  return image
end

function Figure(figure)
  if not FORMAT:match("latex") or not has_class(figure, "allmight-auto-path-caption") then return nil end
  local uri = figure.attr.attributes["data-auto-path-caption-uri"]
  if not uri or pandoc.utils.stringify(figure.caption.long) ~= uri then return nil end
  local human = path_caption(uri)
  if not human then return nil end
  figure.caption.long = pandoc.Blocks({ pandoc.Plain({ pandoc.Str(human) }) })
  return figure
end

function Para(block)
  if not FORMAT:match("latex") or #block.content == 0 then return nil end
  local images = {}
  for _, inline in ipairs(block.content) do
    if inline.t ~= "Image" or not has_class(inline, "allmight-diagram-panel") then return nil end
    images[#images + 1] = inline
  end
  if #images == 1 then
    return pandoc.RawBlock("latex", "\\BookDiagramPanel{" .. images[1].src .. "}")
  end

  local out = {}
  local caption = pandoc.utils.stringify(images[1].caption)
  if caption ~= "" then
    out[#out + 1] = "\\par\\medskip\\noindent\\textbf{" .. latex_escape(caption) .. "}\\par"
  end
  for index, image in ipairs(images) do
    local attributes = image.attr.attributes
    local panel_index = tonumber(attributes["data-panel-index"])
    local panel_count = tonumber(attributes["data-panel-count"])
    if panel_index ~= index or panel_count ~= #images then
      error("Invalid ordered panel metadata for diagram image")
    end
    -- Generated bundle paths contain only relative path separators and a
    -- SHA256 filename, so they are safe as the braced argument to the macro.
    out[#out + 1] = "\\par\\noindent\\textit{Panel " .. panel_index .. "/" .. panel_count .. "}\\par\\BookDiagramPanel{" .. image.src .. "}\\par"
  end
  return pandoc.RawBlock("latex", table.concat(out, "\n"))
end
