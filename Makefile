NAME	= webserv

SRCS	= $(wildcard src/*.cpp)

OBJDIR	= obj
OBJS	= $(SRCS:src/%.cpp=$(OBJDIR)/%.o)

CXX = c++
CXXFLAGS = -std=c++98 -I includes/ -Wall -Wextra -Werror

all:	$(NAME)

$(NAME): $(OBJS)
	@$(CXX) $(CXXFLAGS) -o $(NAME) $(OBJS)
	@echo "Build of $(NAME) completed."

$(OBJDIR)/%.o: src/%.cpp | $(OBJDIR)
	@echo "Compiling $<..."
	@$(CXX) $(CXXFLAGS) -c -o $@ $<

$(OBJDIR):
	@mkdir -p $(OBJDIR)

clean:
	@rm -rf $(OBJDIR)
	@echo "Clean of ${NAME} completed."

fclean: clean
	@rm -f $(NAME)
	@rm -rf $(OBJDIR)
	@echo "Full clean of ${NAME} completed."

re: fclean all
	@echo "Rebuild of ${NAME} completed."

.PHONY: all clean fclean re