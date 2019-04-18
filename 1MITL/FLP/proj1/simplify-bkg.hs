-- Project: simplify-bkg
-- Author: David Bolvansky
-- Login: xbolva00

import System.Environment -- getArgs
import Data.List -- intersection, union, nub
import Control.Exception -- catch IO error

-- Data types
type Sym = Char
type SymList = [Sym]
type TermList = SymList
type NonTermList = SymList
type InitSym = Sym

-- Represents a rule in a CFG
data CFGRule = CFGRule Sym SymList deriving (Eq,Show)
type CFGRuleList = [CFGRule]

-- Represents a context free grammar (CFG)
data CFG = CFG NonTermList TermList CFGRuleList InitSym deriving (Show)

-- Getters
getLeftSide :: CFGRule -> Sym
getLeftSide (CFGRule l r) = l
getRightSide :: CFGRule -> SymList
getRightSide (CFGRule l r) = r
getLeftSidesOfRules :: CFGRuleList -> [Sym]
getLeftSidesOfRules xs = map getLeftSide xs
getRightSidesOfRules :: CFGRuleList -> [Sym]
getRightSidesOfRules [] = []
getRightSidesOfRules (x:xs) = getRightSide x ++ getRightSidesOfRules xs

-- Converters
strToRules :: [String] -> [CFGRule]
strToRules s = map (\x -> CFGRule (head x) (drop 3 x)) s
strToSymbols :: String -> [Sym]
strToSymbols s = filter (not . (\c -> elem c ", ")) s
symbolsWithCommas :: [Sym] -> String
symbolsWithCommas []     = []
symbolsWithCommas (x:[]) = [x]
symbolsWithCommas (x:xs) = x:',':symbolsWithCommas xs

-- Printers
printSymbols :: [Sym] -> IO ()
printSymbols s = putStrLn (symbolsWithCommas (sort s))
printInitSymbol :: Char -> IO ()
printInitSymbol s = putStrLn [s]
printRule :: CFGRule -> IO ()
printRule (CFGRule l r) = putStrLn (l:("->" ++ r))
-- Prints a nonempty list of rules
printRules :: CFGRuleList -> IO ()
printRules []  = return ()
printRules (x:xs)  = do
    printRule x
    printRules xs
-- Prints a CFG
printGrammar :: CFG -> IO ()
printGrammar (CFG n t p s) = do
    printSymbols n
    printSymbols t
    printInitSymbol s
    printRules p

-- Matchers
isTermSym :: Char -> Bool
isTermSym = \c -> elem c ['a'..'z']
isNonTermSym :: Char -> Bool
isNonTermSym = \c -> elem c ['A'..'Z'] 
isEpsilon :: Char -> Bool
isEpsilon = (== '#')
matchRightSide :: [Sym] -> [Sym] -> Bool
matchRightSide rightSide symbols = isEpsilon (rightSide !! 0) || all (\c -> elem c symbols) rightSide

-- Computes nonterminals generating terminal strings
computeNt t p 0 = []
computeNt t p n = do
    let prevNt = computeNt t p (n-1)
    let ntWithT = union prevNt t
    let matchedRules = filter (\r -> matchRightSide (getRightSide(r)) ntWithT) p
    nub (getLeftSidesOfRules matchedRules)

-- Computes reachable symbols
computeV p s 0 = [s]
computeV p s n = do
    let prevV = computeV p s (n-1)
    let matchedRules = filter (\r -> elem (getLeftSide r)  prevV) p
    union (nub (getRightSidesOfRules matchedRules)) prevV

-- Algorithm steps
firstAlgoStep :: CFG -> Either String CFG
firstAlgoStep (CFG n t p s) = do
    let nt = computeNt t p (length n)
    if (notElem s nt)
    then 
        Left "The language of grammar is empty."
    else do
        let ntWithT = union nt t
        let n' = union nt [s] 
        let p' = filter (\r -> elem (getLeftSide r) nt && matchRightSide (getRightSide(r)) ntWithT) p
        Right (CFG n' t p' s)

secondAlgoStep :: CFG -> CFG
secondAlgoStep (CFG n t p s) = do
    let v = computeV p s (length n)
    let n' = intersect n v
    let t' = intersect t v
    let p' = filter (\r -> elem (getLeftSide r) v && matchRightSide (getRightSide(r)) v) p
    CFG n' t' p' s

-- Checks syntax and semantic correctness of CFG
checkRules :: [String] -> Bool
checkRules [] = True
checkRules (x:xs) = (length x >= 4 && take 2 (drop 1 x) == "->") && checkRules xs
checkInitSym :: String -> Bool
checkInitSym s = length s == 1 && isNonTermSym(head s)
checkSeparators :: String -> Bool
checkSeparators l = l == symbolsWithCommas (strToSymbols l)
checkSyntax :: [String] -> Bool
checkSyntax inputLines = length inputLines >= 4 && checkSeparators (inputLines !! 0) && 
                              checkSeparators (inputLines !! 1) && checkInitSym(inputLines !! 2) &&
                              checkRules (drop 3 inputLines)

checkGrammar :: CFG -> Either String CFG
checkGrammar grammar@(CFG n t p s)
    | n /= nub n =
        Left "Duplicated nonterminals."
    | t /= nub t =
        Left "Duplicated terminals."
    | p /= nub p =
        Left "Duplicated rules."
    | any (not . \s -> isNonTermSym(s)) n =
        Left "Invalid symbol in list of nonterminals."
    | any (not . \s -> isTermSym(s)) t =
        Left "Invalid symbol in list of terminals."
    | any (not . \r -> isNonTermSym(getLeftSide r)) p = 
        Left "Left side of rule is not a nonterminal symbol."
    | any (not . \r -> elem (getLeftSide r) n) p  =
        Left "Unknown symbol on the left side of rules."
    | any (not . \r -> matchRightSide (getRightSide(r)) (union n t)) p =
        Left "Unknown symbol on the right side of rules."
    | any (\r -> length (getRightSide(r)) > 1 && isEpsilon(getRightSide(r) !! 0)) p =
        Left "Unknown symbol after epsilon on the right side of rules."
    | not (elem s n)  = 
        Left "Initial grammar symbol is not contained in the nonterminal list"
    | otherwise = 
        Right grammar

-- Parses input to internal CFG representation
parseGrammar :: [String] -> CFG
parseGrammar (ns:ts:ss:rs) = CFG (strToSymbols ns) (strToSymbols ts) (strToRules rs) (head ss)

loadGrammar :: String -> Either String CFG
loadGrammar input = do
    let inputLines = dropWhile (=="") (lines input)
    if not(checkSyntax inputLines)
    then 
        Left "Invalid input grammar."
    else 
        Right (parseGrammar inputLines)

-- Reads file or standard input stream
readInput :: [FilePath] -> IO String
readInput args
    | length args == 2 = readFile (args !! 1)
    | otherwise = getContents

main :: IO ()
main = do
  args <- getArgs
  if length args < 1 || length args > 2
  then 
    error "Invalid number of arguments. Run as: simplify-bkg option [input]"
  else do
    let opt = head args
    if notElem opt ["-i", "-1", "-2"]
    then 
        error "Invalid option. Run as: simplify-bkg option [input]"
    else do
        input <- (catch :: IO a -> (IOError -> IO a) -> IO a) (readInput args)
            (\_ -> error "File not found.")
        case loadGrammar input of
            Left syntaxError -> error syntaxError
            Right grammar -> do
                case checkGrammar grammar of
                    Left grammarErr -> error grammarErr
                    Right grammar -> do
                        case (opt) of
                            "-i" -> printGrammar grammar
                            "-1" -> do
                                    case firstAlgoStep grammar of 
                                        Left emptyLG -> error emptyLG
                                        Right grammar -> printGrammar grammar
                            "-2" -> do
                                    case firstAlgoStep grammar of 
                                        Left emptyLG -> error emptyLG
                                        Right grammar -> printGrammar (secondAlgoStep grammar)  