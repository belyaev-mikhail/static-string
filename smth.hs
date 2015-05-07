



tokenize ""         ""      acc = acc
tokenize ""         currSym acc = currSym:acc
tokenize (' ':rest) ""      acc = tokenize rest "" acc
tokenize (' ':rest) currSym acc = tokenize rest "" (currSym:acc)
tokenize ('(':rest) ""      acc = tokenize rest "" ("(":acc)
tokenize (')':rest) ""      acc = tokenize rest "" (")":acc)
tokenize ('(':rest) currSym acc = tokenize ('(':rest) "" (currSym:acc)
tokenize (')':rest) currSym acc = tokenize (')':rest) "" (currSym:acc)
tokenize (ih: rest) currSym acc = tokenize rest (currSym ++ [ih]) acc 
