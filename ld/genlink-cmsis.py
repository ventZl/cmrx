from __future__ import print_function
from pprint import pprint
import sys
import os
import math
import argparse

INIT = 0
BLOCK_COMMENT = 1

NONE = 0
SLASH_CANDIDATE = 1
BLOCK_COMMENT_START = 2
BLOCK_COMMENT_END = 3
SYMBOL_NAME = 4
SLASH = 5
LINE_COMMENT_START = 6
WHITE_SPACE = 7
COLON = 8
SEMICOLON = 9
FULLSTOP = 10
COMMA = 11
NEWLINE = 12
LEFT_BRACKET = 13
RIGHT_BRACKET = 14
LEFT_SHARP_BRACKET = 15
RIGHT_SHARP_BRACKET = 16
LEFT_CURLY_BRACKET = 17
RIGHT_CURLY_BRACKET = 18
ASSIGN = 19
COMPARE = 20
MULTIPLY = 21
ADD = 22
SUBTRACT = 23
INCREMENT = 24
DECREMENT = 25
NUMBER = 26
SYMBOL_GREATER_THAN = 27
SYMBOL_LESS_THAN = 28
PIPE_INTO = 29
PIPE_FROM = 30
STRING_LITERAL = 31
APOSTROPHE = 32
QUESTION_MARK = 33
EOF = 34
SYMBOL_GREATER_THAN_EQUAL = 35
SYMBOL_LESS_THAN_EQUAL = 36
FILE_NAME = 37
RAW_SYMBOL_NAME = 38
SECTION_NAME = 39
FILLER = 40
SECTION_CANDIDATE = 41
COMMENT = 42
IN_LINE_COMMENT = 43
IN_BLOCK_COMMENT = 44
AMPERSAND = 45
EXCLAMATION_MARK = 46
TEXT = 47

def filename_to_libname(file_name):
    '''
    Extract library name from filename. Removes "lib" prefix, if present.
    '''
    lib_name = os.path.basename(file_name).split('.', 2)[0]
    if (lib_name[0:3] == "lib"):
        lib_name = lib_name[3:]

    return lib_name

def sort_by_size(container):
    '''
    Sorts container by size descending. This stuff should not really be here,
    rather in MPUAllocation class.
    '''
    out = []
    for k in container.keys():
        out.append((k, container[k]))

    out.sort(key = lambda item : item[1].get_size(), reverse = True)

    return out

class Token:
    '''
    Lexical token. Holds token type and its value.
    Value is held so that the input can be reconstructed from a list of tokens.
    '''
    def __init__(self, type = NONE, value = ""):
        self.type = type
        self.value = value

    def append(self, token):
        self.value += token.value

    def clone(self):
        return Token(self.type, self.value)

    def __desc__(self):
        return self.__str__()

class TokenList:
    '''
    Container storing runs of tokens. Allows some operations on this list,
    such as dumping it, token type pattern matching, finding pair braces,
    creation of sub-lists.
    '''
    def __init__(self, tokens = [], begin = None, end = None, parent = None):
        if (tokens != [] and parent != None):
            print("Invalid initialization of token list. Both tokens and parent provided")

        self._tokens = tokens
        self._begin = begin
        self._end = end
        self._parent = parent

    def tokens(self):
        '''
        Return tokens
        '''
        if (self._parent is None):
            return self._tokens
        else:
            return self._parent[self._begin, self._end]

    def delete(self, index):
        if (self._parent is None):
            if (index >= len(self)):
                raise RangeError("Index out of range")
            else:
                self._parent.delete(index + self._begin)
                self._end = self._end - 1
        else:
            if (index > (self._end - self._begin)):
                raise RangeError("Index out of range")
            else:
                del self._tokens[index]

    def append(self, token):
        '''
        Append new token at the end of run
        '''
        if (self._parent is not None):
            self._parent.append(token)
            self._end += 1
        else:
            self._tokens.append(token)

    def __len__(self):
        if (self._parent is None):
            return len(self._tokens)
        else:
            return self._end - self._begin

    def __getitem__(self, key):
        if (self._parent is None):
            return self._tokens[key]
        else:
            if (key > (self._end - self._begin)):
                return None
            return self._parent[self._begin + key]

    def match_pattern(self, iterator, pattern):
        '''
        Will start at offset determined by `iterator` and try to match token types based on types
        provided in `pattern`. The order of types of tokens presented in token list after file
        was parsed must be the same as in `pattern`. Then this function will return true. Otherwise
        it returns false. If token list ends prematurely, then result is also false.
        '''
        for q in range(len(pattern)):
            if ((iterator + q + 1) >= len(self)):
                return False
            if (self[iterator + q + 1].type != pattern[q]):
                return False

        return True

    def dump_pattern(self, iterator, pattern):
        for q in range(len(pattern)):
            print(self[iterator + q + 1].value, end = "")

        print("")

    def dump(self):
        if (len(self) == 0):
            print("(empty)")
            
        for q in range(len(self)):
            token = self[q]
            if (token.type == FILE_NAME):
                print ("<%s>" % (token.value), end = "")
            elif (token.type == SECTION_NAME):
                print ("[%s]" % (token.value), end = "")
            elif (token.type == SYMBOL_NAME):
                print ("{%s}" % (token.value), end = "")
            elif (token.type == FILLER):
                print ("/%s/" % (token.value), end = "")
            elif (token.type == COMMENT):
                print ("#%s#" % (token.value), end = "")
            else:
                print ("%s" % (token.value), end = "")

    def join(self):
        out = ""
        for token in self:
            out = out + token.value

        return out


    def _do_find_pair(self, position, direction, opening, closing):
        stack = 1
        while position < len(self) and position >= 0:
            if (self[position].type == opening):
                stack += 1
            elif (self[position].type == closing):
                stack -= 1


            if (stack == 0):
                # This was the pair token for one searched originally
                return position

            position += direction
        return None


    def find_pair(self, iterator):
        '''
        Find position of pair bracket. Will examine position given by iterator
        and find closing bracket for given opening bracket. If there are any
        nested opening brackets before matching closing bracket is reached, their
        matchin closing brackets are skipped.
        Returns position of closing bracket.
        '''
        opening = self[iterator].type
        closing = None
        position = iterator + 1
        if (opening == LEFT_BRACKET):
            closing = RIGHT_BRACKET
        elif (opening == LEFT_CURLY_BRACKET):
            closing = RIGHT_CURLY_BRACKET
        elif (opening == LEFT_SHARP_BRACKET):
            closing = RIGHT_SHARP_BRACKET
        else:
            # token at opening position is not a valid pair token
            return None
        return self._do_find_pair(position, +1, opening, closing)

    def find_pair_reverse(self, iterator):
        ''' 
        same as fdind_pair, but acts on right brackets and searches backwards
        '''
        opening = self[iterator].type
        closing = None
        position = iterator - 1
        if (opening == RIGHT_BRACKET):
            closing = LEFT_BRACKET
        elif (opening == RIGHT_CURLY_BRACKET):
            closing = LEFT_CURLY_BRACKET
        elif (opening == RIGHT_SHARP_BRACKET):
            closing = LEFT_SHARP_BRACKET
        else:
            # token at opening position is not a valid pair token
            return None

        return self._do_find_pair(position, -1, opening, closing)


    def sub_range(self, begin, end):
        '''
        Returns sub-range of token run determined by start and end position. sub-range
        is created in <begin, end) fashion, thus token at begin position is included,
        but token at end position is not.
        '''

#        print("Sub range created on interval <%d, %d)" % (begin, end))
        if (end < begin):
            return None
        if (end >= len(self)):
            return None
        return TokenList([], begin, end, self)

    def find_next(self, pos, token_type):
        while pos < len(self):
            if (self[pos].type == token_type):
                return pos

            pos+= 1

        return None

    def find_prev(self, pos, token_type):
        while pos >= 0:
            if (self[pos].type == token_type):
                return pos

            pos -= 1

        return None

    def insert(self, pos, run):
        if (self._parent is None):
#            print("Inserting run of %d tokens here" % (len(run)))
#            pprint(self._tokens)
            for q in range(len(run)):
                self._tokens.insert(pos + q, run[q])
        else:
#            print("Inserting run of %d tokens into parent" % (len(run)))
            self._parent.insert(self._begin + pos, run)
            self._end += len(run)
#            self.dump()

    def copy(self):
        out = TokenList([])
        for token in self:
            out.append(Token(token.type, token.value))

        return out

class LinkerFile(TokenList):
    def __init__(self, tl):
        TokenList.__init__(self, tl._tokens)

    def same(self, other):
        if (len(self) != len(other)):
            return False

        for q in range(len(self)):
            if (self[q].type != other[q].type):
                return False

            if (self[q].value != other[q].value):
                return False

        return True

    def find_current_section_name(self, block, position):
        while position >= 0:
            if (block[position].type == WHITE_SPACE):
                position -= 1
                continue

            if (block[position].type == RIGHT_BRACKET):
                position = block.find_pair_reverse(position)
                if (position is None):
                    # sum-ting-wong
                    return None
                position -= 1
                continue

            if (block[position].type == SECTION_NAME):
                return position

            position -= 1

    def _indent_seq(self, include_stmt, indent_by):
        for q in reversed(range(len(include_stmt))):
            if (include_stmt[q].type == NEWLINE and q < len(include_stmt) - 1):
#                print("Newline found at offset %d, indenting by '%s'" % (q, indent_by.value))
                include_stmt.insert(q + 1, indent_by)
        include_stmt.insert(0, indent_by)

        return include_stmt

    def place_include_into_section(self, section, include_stmt):
        for q in range(len(section)):
            if (section[q].type == MULTIPLY or (section[q].type == SYMBOL_NAME and section[q].value == "KEEP")):
                # Here is the first something, which places input sections into output sections
                # Find previous whitespace to use it as a guide for indentation
                whitespace = section.find_prev(q, WHITE_SPACE)
                newline = section.find_prev(q, NEWLINE)
                include_stmt = self._indent_seq(include_stmt, Token(WHITE_SPACE, section[whitespace].value))
                section.insert(newline + 1, include_stmt)
                return
        pass

    def place_include_into_section_end(self, section, include_stmt):
        for q in reversed(range(len(section))):
            if (section[q].type == MULTIPLY or (section[q].type == SYMBOL_NAME and section[q].value == "KEEP")):
                # Find previous whitespace to use it as a guide for indentation
                whitespace = section.find_prev(q, WHITE_SPACE)
                # Find next newline to guide statements placement
                newline = section.find_next(q, NEWLINE)
                include_stmt = self._indent_seq(include_stmt, Token(WHITE_SPACE, section[whitespace].value))
#                include_stmt.insert(0, Token(WHITE_SPACE, section[whitespace].value))
                section.insert(newline + 1, include_stmt)
                return

    def _process_sections_block(self, block, binary_name):
        section_start_pattern = [ COLON ]
        pos = 0
        while pos is not None and pos < len(block):
            if (block.match_pattern(pos, section_start_pattern)):
                if (pos is None):
                    # We haven't found anything, exit
                    break
                section_name = self.find_current_section_name(block, pos)
                prev_newline = block.find_prev(pos, NEWLINE)
                indentation = "\t"
                if (block[prev_newline + 1].type == WHITE_SPACE):
                    indentation = block[prev_newline + 1].value

                if (section_name is not None):
                    # Determine section boundaris and generate sub-block covering section
                    # definition. Useful for in-section modifications
                    section_opening = block.find_next(pos, LEFT_CURLY_BRACKET)
                    if (section_opening is not None):
                        section_closing = block.find_pair(section_opening)
                        section_definition = block.sub_range(section_opening, section_closing + 1)
                    else:
                        # Nope? Section closing bracket not found?
                        pos += 1
                        continue

                    # Find out the destination memory region for this section
                    output_region_pos = self._get_section_output_region(block, section_closing)

                    # Then find next newline, which is position where we append our 
                    # newly generated stuff
                    next_newline_pos = block.find_next(output_region_pos, NEWLINE)

                    if (block[section_name].value == ".data" or block[section_name].value == ".bss"):
                        if (block[section_name].value == ".data"):
                            include_seq = self._gen_include("gen." + binary_name + ".data.ld")
                        else:
                            include_seq = self._gen_include("gen." + binary_name + ".bss.ld")
                            
                            # We have to do this before we extend .bss, otherwise our iterators 
                            # would be invalid.

                            output_region_pos = self._get_section_output_region(block, section_closing)

                            shared_section_seq = self._gen_section(".shared", block[output_region_pos].value,
                                    self._gen_include("gen." + binary_name + ".shared.ld"), indentation)

                            block.insert(next_newline_pos + 1, shared_section_seq)
                        
                        self.place_include_into_section(section_definition, include_seq)

                    elif (block[section_name].value == ".text"):
                        indent = Token(WHITE_SPACE, indentation)
                        include_seq = self._gen_include("gen." + binary_name + ".text.ld")
                        include_seq += self._gen_comment("Compile-time process description block")
                        include_seq += self._gen_alignment(4)
                        include_seq += self._gen_variable_assignment("__applications_start", [Token(FULLSTOP, ".")])
                        include_seq += self._gen_keep_deploy("*", [".applications"])
                        include_seq += self._gen_variable_assignment("__applications_end", [Token(FULLSTOP, ".")])
                        include_seq += self._gen_comment("Compile-time thread auto-create block")
                        include_seq += self._gen_variable_assignment("__thread_create_start", [Token(FULLSTOP, ".")])
                        include_seq += self._gen_keep_deploy("*", [".thread_create"])
                        include_seq += self._gen_variable_assignment("__thread_create_end", [Token(FULLSTOP, ".")])
                        include_seq += self._gen_comment("RPC interface VTABLEs")
                        include_seq += self._gen_include("gen." + binary_name + ".vtable.ld")

                        output_region_pos = self._get_section_output_region(block, section_closing)
                        
                        # Insert some symbols just after .text section definition.
                        # Here, the position does not really matter, but putting the bloc after .text
                        # won't invalidate our iterators.
                        # We have to do this before we extend .text, otherwise our iterators 
                        # would be invalid.

                        flash_details_seq = [ Token(NEWLINE, "\n"), indent ] + self._gen_comment("Publish FLASH base address as a symbol accessible from C")
                        flash_details_seq += [ indent ] + self._gen_variable_assignment("__cmrx_flash_origin",  
                            self._gen_function_wrap("ORIGIN", [ block[output_region_pos].clone() ] )
                            )
                        flash_details_seq += [ indent ] + self._gen_comment("Publish FLASH size as a symbol accessible from C")
                        flash_details_seq += [ indent ] + self._gen_variable_assignment("__cmrx_flash_length", 
                            self._gen_function_wrap("LENGTH", [ block[output_region_pos].clone() ] )
                            )

                        block.insert(next_newline_pos + 1, flash_details_seq)

                        self.place_include_into_section_end(section_definition, include_seq)

                    # We have processed this section, fast forward to its end
                    pos = section_closing


            pos += 1


    def add_subscript_includes(self, binary_name):
        '''
        Will find .data section and augment it to include another script.
        Will add another section .vtable just after .data section.
        Will find .bss section and augment it to include another script.
        '''
        sections_block_pattern = [ SYMBOL_NAME, NEWLINE, LEFT_CURLY_BRACKET ]
        entry_pattern = [ SYMBOL_NAME, LEFT_BRACKET, SYMBOL_NAME, RIGHT_BRACKET ]
        q = 0;
        while q < len(self):
            if (self[q].type == NEWLINE):
                if (self.match_pattern(q, sections_block_pattern) and self[q + 1].value == "SECTIONS"):
                    begin = q + 3
                    end = self.find_pair(begin)
                    block = self.sub_range(begin, end + 1)
                    # We finished processing this block, fast-forward at its end
                    self._process_sections_block(block, binary_name)
                    q = end
                elif (self.match_pattern(q, entry_pattern) and self[q + 1].value == "ENTRY"):
                    inst_seq = self._gen_include("gen." + binary_name + ".inst.ld")
                    self.insert(q + 6, inst_seq)

            q += 1

    def _get_section_output_region(self, block, section_end_pos):
        # Find first > after } which closes current section 
        # then find next symbol name. This is our destination memory region
        redir_pos = block.find_next(section_end_pos, SYMBOL_GREATER_THAN)
        if (redir_pos is None):
            return None
        segment_pos = block.find_next(redir_pos, SYMBOL_NAME)
        if (segment_pos is None):
            return None

        return segment_pos

    def _gen_function_wrap(self, function_name, content):
        function_seq = [ Token(SYMBOL_NAME, function_name), Token(LEFT_BRACKET, "(") ]
        function_seq += content + [ Token(RIGHT_BRACKET, ")") ]
        return function_seq

    def _gen_section(self, name, out_region, content, indentation):
        section_seq = [ Token(NEWLINE, "\n"), Token(WHITE_SPACE, indentation), Token(SECTION_NAME, name), Token(WHITE_SPACE, " "), 
            Token(COLON, ":"), Token(WHITE_SPACE, " "), Token(LEFT_CURLY_BRACKET, "{"), Token(NEWLINE, "\n") ]
        section_seq += [ Token(WHITE_SPACE, indentation + indentation) ] + content
        section_seq += [ Token(WHITE_SPACE, indentation), Token(RIGHT_CURLY_BRACKET, "}"), 
            Token(WHITE_SPACE, " "), Token(SYMBOL_GREATER_THAN, ">"), Token(WHITE_SPACE, " "), 
            Token(SYMBOL_NAME, out_region), Token(NEWLINE, "\n") ]
        return section_seq

    def _gen_keep_deploy(self, file_name, sections):
        seq = [ Token(SYMBOL_NAME, "KEEP"), Token(LEFT_BRACKET, "("), Token(FILE_NAME, "*"), 
            Token(LEFT_BRACKET, "(") ]
        whitespace = False
        for section in sections:
            if (whitespace):
                seq += [ Token(WHITE_SPACE, " ")]

            whitespace = True
            seq += [ Token(SECTION_NAME, section)]

        seq = seq + [ Token(RIGHT_BRACKET, ")"), Token(RIGHT_BRACKET, ")"), Token(NEWLINE, "\n") ]
        return seq;

    def _gen_include(self, file_name):
        return [ Token(SYMBOL_NAME, "INCLUDE"), Token(WHITE_SPACE, " "), Token(FILE_NAME, file_name), Token(NEWLINE, "\n") ]

    def _gen_comment(self, text):
        return [ Token(COMMENT, "/* " + text + " */"), Token(NEWLINE, "\n") ]

    def _gen_variable_assignment(self, variable_name, value_list):
        ''' Will generate output in the form of <variable_name> = { <values_in_value_list>}
        '''
        space = Token(WHITE_SPACE, " ")
        out = [ Token(SYMBOL_NAME, variable_name), space, Token(ASSIGN, "="), space ]
        out += value_list
        out += [ Token(SEMICOLON, ";"), Token(NEWLINE, "\n") ]
        return out

    def _gen_library_inclusion(self, library_name, section_name):
        space = Token(WHITE_SPACE, " ")
        return [ Token(FILE_NAME, library_name), Token(LEFT_BRACKET, "("), Token(SECTION_NAME, "."+section_name), 
            space, Token(SECTION_NAME, "."+section_name+".*"), Token(RIGHT_BRACKET, ")"), Token(NEWLINE, "\n") ]

    def _gen_alignment(self, alignment):
        space = Token(WHITE_SPACE, " ")
        return [ Token(FULLSTOP, "."), space, Token(ASSIGN, "="), space, Token(SYMBOL_NAME, "ALIGN"), 
                Token(LEFT_BRACKET, "("), Token(NUMBER, "0x%X" % alignment), Token(RIGHT_BRACKET, ")"),
                Token(SEMICOLON, ";"), Token(NEWLINE, "\n") ]

    def _gen_extern(self, symbol_name):
        return [ Token(SYMBOL_NAME, "EXTERN"), Token(LEFT_BRACKET, "("), Token(SYMBOL_NAME, symbol_name), 
                Token(RIGHT_BRACKET, ")"), Token(NEWLINE, "\n") ]

    def add_library_sections_aligned(self, file_name, section, alignment):
        lib_name = filename_to_libname(file_name)
        whitespace="\t\t"
        tab = Token(WHITE_SPACE, "\t\t")
        lib_seq = self._gen_comment("Process name: "+lib_name)
        lib_seq += [ tab ] + self._gen_alignment(alignment)
        lib_seq += [ tab ] + self._gen_variable_assignment(lib_name+"_"+section+"_start", [ Token(FULLSTOP, ".") ] )
        lib_seq += [ tab ] + self._gen_library_inclusion(file_name, section)
        lib_seq += [ tab ] + self._gen_alignment(alignment)
        lib_seq += [ tab ] + self._gen_variable_assignment(lib_name+"_"+section+"_end", [ Token(FULLSTOP, ".") ] )
        self.insert(len(self), lib_seq)

    def add_extern_declaration(self, file_name):
        lib_name = filename_to_libname(file_name)
        tab = Token(WHITE_SPACE, "\t\t")
        extern_seq = self._gen_comment("Process name: "+lib_name)
        extern_seq = extern_seq + [ tab ] + self._gen_extern(lib_name+"_instance")
        self.insert(len(self), extern_seq)

class MPUAllocation:
    def __init__(self, owner):
        self._owner = owner
        self.size = 0
        self.fill = 0

    def owner(self):
        return self._owner

    def add_alloc(self, size):
        if self.fill > 0:
            self.size = self.size + self.fill
            self.fill = 0

        self.size = self.size + size

    def add_fill(self, fill):
        if self.fill > 0:
            self.size = self.size + self.fill
        
        self.fill = fill

    def get_size(self):
        return self.size

    def get_alignment(self):
        if (self.get_size() == 0):
            return 4
        align = int(math.pow(2, self.get_size().bit_length()))
        if align < 0x100:
            align = 0x100

        return align

    def __str__(self):
       return "0x%04X allocated" % self.get_size()

    def __desc__(self):
        return self.__str__()

class MapFile(TokenList):
    def __init__(self, token_list):
        self._tokens = token_list._tokens
        self._begin = token_list._begin
        self._end = token_list._end
        self._parent = token_list._parent
        self._sections = { '.data' : {}, '.bss' : {}, '.shared' : {}, '.text': {}, '.rodata' : {}}

    def process(self):
        '''
        Will search for unindented output section name (.data, .text, .bss, .shared...)
        Then, it will search for patterns determining explicit placement of input file's
        sections (<file_name>(.<section> .<section>.*) pattern.
        Then records of all sections put into output after this directive, including
        any potential fillers are collected.
        Implicit section placement (*(.<section>)) cancels the effect of explicit input
        file's section placement and terminates search for section records.
        '''
        pattern_output_section = [ SECTION_NAME, WHITE_SPACE, NUMBER, WHITE_SPACE, NUMBER ]
        pattern_file = [ WHITE_SPACE, FILE_NAME, LEFT_BRACKET, SECTION_NAME, WHITE_SPACE, SECTION_NAME, RIGHT_BRACKET ]
        pattern_file_alt = [ WHITE_SPACE, FILE_NAME, LEFT_BRACKET, SECTION_NAME, RIGHT_BRACKET ]
        pattern_section = [ WHITE_SPACE, SECTION_NAME, WHITE_SPACE, NUMBER, WHITE_SPACE, NUMBER, WHITE_SPACE, FILE_NAME ]
        pattern_section_broken = [ WHITE_SPACE, SECTION_NAME, NEWLINE, WHITE_SPACE, NUMBER, WHITE_SPACE, NUMBER, WHITE_SPACE, FILE_NAME ]
        pattern_fill = [ WHITE_SPACE, FILLER, WHITE_SPACE, NUMBER, WHITE_SPACE, NUMBER ]
        pattern_terminator = [ WHITE_SPACE, MULTIPLY, LEFT_BRACKET, SECTION_NAME, RIGHT_BRACKET ]

        input_section = None
        input_file = None

        tokens = self.tokens()

        for q in range(len(tokens)):
            if (tokens[q].type == NEWLINE):
                if (self.match_pattern(q, pattern_output_section)):
                    # Output section begins
                    input_section = tokens[q + 1].value
                    address = tokens[q + 3].value
                    size = tokens[q + 5].value
#                    print("Section `%s` found!" % (input_section))
                if (self.match_pattern(q, pattern_file) or self.match_pattern(q, pattern_file_alt)):
                    # Input file matches
                    section = tokens[q + 4].value
                    if (section != input_section):
#                        print("Section expected to be `%s` but `%s` found" % (input_section, section))
                        continue
                    input_file = tokens[q + 2].value
                    self._get_file_section(input_file, section)
                if (self.match_pattern(q, pattern_section)):
                    address = tokens[q + 4].value
                    size = tokens[q + 6].value
                    file = tokens[q + 8].value
                    if (file != input_file):
#                        print("File expected to be `%s` but `%s` found" % (input_file, file))
                        continue
                    self.add_file_alloc(input_file, input_section, address, size)
                if (self.match_pattern(q, pattern_section_broken)):
                    address = tokens[q + 5].value
                    size = tokens[q + 7].value
                    file = tokens[q + 9].value
                    if (file != input_file):
#                        print("File expected to be `%s` but `%s` found" % (input_file, file))
                        continue
                    self.add_file_alloc(input_file, input_section, address, size)
                if (self.match_pattern(q, pattern_fill)):
                    address = tokens[q + 4].value
                    size = tokens[q + 6].value
                    self.add_filler(input_file, input_section, address, size)
                if (self.match_pattern(q, pattern_terminator)):
                    input_file = None

    def _get_file_section(self, file_name, section):
        lib_name = filename_to_libname(file_name)
        if (section not in self._sections):
            print("Section %s is not a legal section!" % (section))
            os.abort()

        if (file not in self._sections[section]):
            self._sections[section][lib_name] = MPUAllocation(file_name)

        return self._sections[section][lib_name]

    def add_file_alloc(self, file, section, base, size):
        if (file is None):
            return

        section = self._get_file_section(file, section)
        section.add_alloc(int(size, 16))
        
#        print("%s(%s) %s:%s" % (file, section, base, size))

    def add_filler(self, file, section, base, size):
        if (file is None):
            return

        section = self._get_file_section(file, section)
        section.add_fill(int(size, 16))

#        print("%s(%s) %s:%s ***" % (file, section, base, size))
        pass

    def mpu_blocks(self, section):
        return self._sections[section]

class Parser:
    def __init__(self):
        pass 

    def undo_read(self, token, file):
        file.seek(-1, 1)
        token.value = token.value[:-1]

    def parse_error(self, token, character):
        '''
        Handle parse error.
        While we are not going to do full syntactic analysis of the linker script, we will simply
        allow ourselves to bail out here with "text" token being "recognized".
        '''
        token.type = TEXT
        return token

    def token(self, file):
        '''
        Lexical parser. Will parse out next token and return its type and content.
        '''
        token = Token()
        while True:
#            print("Token before read (%d, \"%s\")"  % (token.type, token.value))
            c = file.read(1)
#            print("Read: '%c'" % (c))
            if (c == '' and token.type != NONE):
                return token

            token.value += c
            if (token.type == NONE):
                if (c == "/"):
                    token.type = SLASH_CANDIDATE
                elif ((c >= 'A' and c <= 'Z') or (c >= 'a' and c <= 'z') or c == '_'):
                    token.type = SYMBOL_NAME
                elif (c == ' ' or c == '\t'):
                    token.type = WHITE_SPACE
                elif (c == ':'):
                    token.type = COLON
                    return token
                elif (c == ';'):
                    token.type = SEMICOLON
                    return token
                elif (c == '.'):
                    token.type = FULLSTOP
                    return token
                elif (c == ','):
                    token.type = COMMA
                    return token
                elif (c == '\r'):
                    token.type = NEWLINE
                elif (c == '\n'):
                    token.type = NEWLINE
                    return token
                elif (c == '('):
                    token.type = LEFT_BRACKET
                    return token
                elif (c == ')'):
                    token.type = RIGHT_BRACKET
                    return token
                elif (c == '['):
                    token.type = LEFT_SHARP_BRACKET
                    return token
                elif (c == ']'):
                    token.type = RIGHT_SHARP_BRACKET
                    return token
                elif (c == '{'):
                    token.type = LEFT_CURLY_BRACKET
                    return token
                elif (c == '}'):
                    token.type = RIGHT_CURLY_BRACKET
                    return token
                elif (c == '='):
                    token.type = ASSIGN
                elif (c == '*'):
                    token.type = MULTIPLY
                elif (c >= '0' and c <= '9'):
                    token.type = NUMBER
                elif (c == '>'):
                    token.type = SYMBOL_GREATER_THAN
                elif (c == '<'):
                    token.type = SYMBOL_LESS_THAN
                elif (c == '-'):
                    token.type = SUBTRACT
                elif (c == '+'):
                    token.type = ADD
                elif (c == '"'):
                    token.type = STRING_LITERAL
                elif (c == '\''):
                    token.type = APOSTROPHE
                    return token
                elif (c == '?'):
                    token.type = QUESTION_MARK
                    return token
                elif (c == '!'):
                    token.type = EXCLAMATION_MARK
                    return token
                elif (c == '&'):
                    token.type = AMPERSAND
                    return token
                elif (c == ''):
                    token.type = EOF
                    return token
                else:
                    token.type = TEXT
                    return token
            elif (token.type == STRING_LITERAL):
                if (c == '"'):
                    return token
            elif (token.type == SUBTRACT):
                if (c == '-'):
                    token.type = DECREMENT
                    return token
                else:
                    self.undo_read(token, file)
                    return token
            elif (token.type == ADD):
                if (c == '+'):
                    token.type = INCREMENT
                    return token
                else:
                    self.undo_read(token, file)
                    return token
            elif (token.type == SYMBOL_GREATER_THAN):
                if (c == '>'):
                    token.type = PIPE_INTO
                    return token
                elif (c == '='):
                    token.type = SYMBOL_GREATER_THAN_EQUAL
                    return token
                else:
                    self.undo_read(token, file)
                    return token
            elif (token.type == SYMBOL_LESS_THAN):
                if (c == '<'):
                    token.type = PIPE_FROM
                    return token
                if (c == '='):
                    token.type = SYMBOL_LESS_THAN_EQUAL
                    return token
                else:
                    self.undo_read(token, file)
                    return token
            elif (token.type == NUMBER):
                if (c >= '0' and c <= '9'):
                    continue
                elif (c == 'x'):
                    if (token.value == "0x"):
                        continue
                    else:
                        return self.parse_error(token, c)
                elif ((c >= 'A' and c <= 'F') or (c >= 'a' and c <= 'f')):
                    if (token.value[0:2] == "0x"):
                        continue
                    else:
                        return self.parse_error(token, c)
                else:
                    self.undo_read(token, file)
                    return token

            elif (token.type == MULTIPLY):
                if (c == '/'):
                    token.type = BLOCK_COMMENT_END
                    return token
                else:
                    self.undo_read(token, file)
                    return token

            elif (token.type == ASSIGN):
                if (c == '='):
                    token.type = COMPARE
                    return token
                else:
                    self.undo_read(token, file)
                    return token
    
            elif (token.type == NEWLINE):
                if (c == '\n'):
                    return token
                else:
                    # Not \n, undo and return plain \r
                    self.undo_read(token, file)
                    return token
            elif (token.type == SLASH_CANDIDATE):
                if (c == '*'):
                    token.type = BLOCK_COMMENT_START
                    return token
                elif (c == '/'):
                    token.type = LINE_COMMENT_START
                    return token
                else:
                    self.undo_read(token, file)
                    token.type = SLASH;
                    return token
            elif (token.type == SYMBOL_NAME):
                if ((c >= 'A' and c <= 'Z') or (c >= 'a' and c <= 'z') or (c >= '0' and c <= '9') or c == '_'):
                    continue
                else:
                    # Seek one character back (unget last read character) 
                    # What we just read is not part of token, "push back into file"
                    # and return previous value.
                    self.undo_read(token, file)
                    return token
            elif (token.type == WHITE_SPACE):
                if (c == ' ' or c == '\t'):
                    continue
                else:
                    self.undo_read(token, file)
                    return token


        pass

    def merge_symbols(self, input):
        '''
        This function goes through token list and merges following runs together
        .symbolname.(.symbolname|.number)* -> section name
        (..)*(/)*symbolname.symbolname -> file name
        *fill* -> filler
    
        So the parser later can deal with smaller amount of more usable tokens. 
        Rest of the input is passed into output unchanged
        '''
        tokens = []
        state = NONE
        temp_token = None
        reserve = []
        for token in input.tokens():
            if (state == NONE):
                if (token.type == FULLSTOP):
                    state = SECTION_NAME
                    temp_token = Token()
                    temp_token.type = SECTION_NAME
                    temp_token.value = token.value
                elif (token.type == SLASH):
                    state = FILE_NAME
                    temp_token = Token()
                    temp_token.type = FILE_NAME
                    temp_token.value = token.value
                elif (token.type == SYMBOL_NAME):
                    state = RAW_SYMBOL_NAME
                    temp_token = token
                elif (token.type == MULTIPLY):
                    state = FILLER
                    temp_token = Token()
                    temp_token.type = FILLER
                    temp_token.value = token.value
                    reserve = [token]
                else:
                    tokens.append(token)
            elif (state == SECTION_NAME):
                #Here only stuff appropriate for .segment.symbol_name notation is permitted
                if (token.type == FULLSTOP):
                    if (temp_token.value[-1] == '.'):
                        # Two consecutive dots, this one is a file name, not a symbol name
                        temp_token.type = FILE_NAME
                    # Anyway, append it
                    temp_token.value += token.value
                elif (token.type == SYMBOL_NAME or token.type == NUMBER or token.type == MULTIPLY):
                    # Perfectly legal, append it
                    temp_token.value += token.value
                elif (token.type == SLASH):
                    # Not legal, this is actually a file name!
                    state = FILE_NAME
                    temp_token.type = FILE_NAME
                    temp_token.value += token.value
                else:
                    if (temp_token.value == "."):
                        # Sole . is not a valid section name. Convert it back to fullstop token
                        temp_token.type = FULLSTOP
                    # Something else, symbol name has ended
                    # Push both symbol name and terminating token
                    state = NONE
                    tokens.append(temp_token)
                    temp_token = None
                    tokens.append(token)
            elif (state == FILE_NAME):
                # Here relative or absolute paths are allowed
                if (token.type == FULLSTOP or token.type == SLASH or token.type == SYMBOL_NAME or token.type == NUMBER or token.type == SUBTRACT):
                    temp_token.value += token.value
                else:
                    state = NONE
                    tokens.append(temp_token)
                    temp_token = None
                    tokens.append(token)
            elif (state == RAW_SYMBOL_NAME):
                if (token.type == FULLSTOP or token.type == SLASH or token.type == SUBTRACT):
                    temp_token.type = FILE_NAME
                    state = FILE_NAME
                    temp_token.value += token.value
                else:
                    state = NONE
                    tokens.append(temp_token)
                    temp_token = None
                    tokens.append(token)
            elif (state == FILLER):
                if (token.type == SYMBOL_NAME and token.value == "fill" and temp_token.value == '*'):
                    temp_token.value += token.value
                    reserve.append(token)
                elif (token.type == MULTIPLY and temp_token.value == "*fill"):
                    temp_token.value += token.value
                    tokens.append(temp_token)
                    temp_token = None
                    state = NONE
                else:
                    for r in reserve:
                        tokens.append(r)
                    tokens.append(token)
                    state = NONE

            else:
                print("Weird internal state %s" % (state))
                os.abort()


        return TokenList(tokens)

    def parse(self, file):
        
        tokens = []
        while True:
            token = self.token(file)
            if (token.type == EOF):
                break
            tokens.append(token)
        return TokenList(tokens)

class LinkerScriptParser(Parser):
    def __init__(self):
        Parser.__init__(self)

    def parse(self, file):
        tokens = Parser.parse(self, file)
        return LinkerFile(self.merge_symbols(tokens))

    def merge_symbols(self, input):
        '''
        Will perform standard token merging and then merge all
        everything in between block comment start/end into one
        COMMENT token and everything after inline comment start
        until end of line also into COMMENT token. Comment start 
        tokens are included into newly created COMMENT token, 
        while the newline, which terminates the inline comment is
        not.
        Returns token list containing merged symbols
        '''
        intermediate = Parser.merge_symbols(self, input)
        temp_token = None
        status = NONE
        output = TokenList([])
        for token in intermediate.tokens():
            if (status == NONE):
                if (token.type == BLOCK_COMMENT_START):
                    temp_token = Token()
                    temp_token.type = COMMENT
                    temp_token.value = token.value
                    status = IN_BLOCK_COMMENT
                elif (token.type == LINE_COMMENT_START):
                    temp_token = Token()
                    temp_token.type = COMMENT
                    temp_token.value = token.value
                    status = IN_LINE_COMMENT
                else:
                    output.append(token)
            elif (status == IN_BLOCK_COMMENT):
                if (token.type == BLOCK_COMMENT_END):
                    temp_token.value += token.value
                    output.append(temp_token)
                    status = NONE
                else:
                    temp_token.value += token.value
            elif (status == IN_LINE_COMMENT):
                if (token == NEWLINE):
                    output.append(temp_token)
                    output.append(token)
                    status = NONE
                else:
                    temp_token.value += token.value
            else:
                print("Weird status %d" % (status))
                os.abort()

        return output

class MapFileParser(Parser):
    def __init__(self):
        Parser.__init__(self)

    def parse(self, file):
        tokens = Parser.parse(self, file)
        return MapFile(self.merge_symbols(tokens))

args = argparse.ArgumentParser(description="Create, connect and update linker scripts that manage memory protection in applications")
args.add_argument('--create', 
        nargs = 3, 
        metavar = ('device_linker_file', 'binary_linker_file', 'binary_name'),
        help="Create binary-specific linker script using device template")
args.add_argument("--add-application", 
        nargs = 3, 
        metavar = ('library_name', 'binary_name', 'linker_script_directory'),
        help="Add application section to binary linker scripts")
args.add_argument("--realign",
        nargs = 3,
        metavar = ('map_file', 'binary_name', 'linker_script_directory'),
        help = "Update alignment of application segments")
todo = args.parse_args()
pprint(sys.argv)
if (todo.create is not None):
    '''
    This file will parse the main linker script, usually specific for some device.
    It will find the .bss and .data sections and will update them to contain includes to 
    external files. These files then contain process-specific directives for explicit
    deployment. This makes use of ARM MPU possible.
    '''
    ifile = open(todo.create[0], "r")

    parser = LinkerScriptParser()
    linker_file = parser.parse(ifile)

    ifile.close()

    linker_file.add_subscript_includes(todo.create[2])
#    linker_file.dump()

    ofile = open(todo.create[1], "w")
    ofile.write(linker_file.join())
    ofile.close()

    # Create empty files we just injected for inclusion into binary linker script.
    # This will ensure, that even without any process added, we will still build.
    sections = [ "bss", "data", "shared", "text", "vtable", "inst" ]
    output_path = os.path.dirname(todo.create[1])

    for section in sections:

        ofile = open(output_path+"/gen."+todo.create[2]+"."+section+".ld", "w")
        ofile.write("")
        ofile.close()
elif (todo.add_application is not None):
    '''
    This branch will go through all the section-specific includes for main
    linker script and add explicit deployment of library into it. One section per 
    file. This deployment is surrounded by _start and _end variables, so the code
    can determine where in the memory the stuff is allocated. Region is also 
    surrounded by alignment directives aligning the block to 256-byte boundary
    as the very purpose of this action is to make this allocation compatible with
    MPU of ARM MCUs.
    '''
    sections = [ "bss", "data", "shared", "text", "vtable", "inst" ]
    for section in sections:
        file_name = todo.add_application[2] + "/gen."+todo.add_application[1]+"."+section+".ld"
        ifile = open(file_name, "r")
        parser = LinkerScriptParser()
        linker_file = parser.parse(ifile)
        ifile.close()

        if (section == "inst"):
            linker_file.add_extern_declaration(todo.add_application[0])
        else:
            linker_file.add_library_sections_aligned(todo.add_application[0], section, 4)

        ofile = open(file_name, "w")
        ofile.write(linker_file.join())
        ofile.close()
elif (todo.realign is not None):
    '''
    This branch will open, parse and inspect the map file.
    This will result in extraction of allocations made by explicit file placements.
    These allocations are then sorted by the size descenging and alignment is calculated
    for each file x section, so that each process' block can be contained by MPU region.

    Resulting file will be compared with the already existing one. If they differ, then
    the binary is deleted as it won't be bootable due to MPU regions misalignment.

    '''
    ifile = open(todo.realign[0], "r")
    parser = MapFileParser()
    map_file = parser.parse(ifile)
    map_file.process()
    ifile.close()

    # Is the binary layout correct? If we had to change linker scripts, then it is not
    binary_layout_correct = True

    sections = [ "bss", "data", "shared", "text" ]
    for section in sections:
        linker_file_name = todo.realign[2] + "/gen." + todo.realign[1] + "." + section + ".ld"
        ifile = open(linker_file_name, "r")
        parser = LinkerScriptParser()
        old_linker_file = parser.parse(ifile)

        content = map_file.mpu_blocks("."+section)
        sort_by_size(content)

        tl = TokenList([])
        linker_file = LinkerFile(tl)

        for application in content:
            mpu_block = content[application]
            linker_file.add_library_sections_aligned(mpu_block.owner(), section, mpu_block.get_alignment())

        ofile = open(linker_file_name, "w")
        ofile.write(linker_file.join())
        ofile.close()

        if (not old_linker_file.same(linker_file)):
            binary_layout_correct = False
        linker_file = None
        tl = None

    if (not binary_layout_correct):
        elf_file_name = todo.realign[0][0: todo.realign[0].rfind('.')]
        os.remove(elf_file_name)
        print("Linker script updated, please rebuild the target...")
    else:
        print("Target up to date")
