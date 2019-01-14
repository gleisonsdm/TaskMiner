library(ggplot2)
library(dplyr)
library(tibble)
library(readr)
library(tidyr)
runtime_data <- read.csv("Desktop/Utils/data.csv")
BENCH <- runtime_data[1:22,1]
BENCH <- cbind(BENCH,(runtime_data[1:22,6:9] / runtime_data[1:22,2:5]))
metric = gather(BENCH, key='id', value='value', 2, 3, 4, 5)
p = ggplot() + geom_boxplot(data= metric, aes(BENCH, log(value)), colour='black') + coord_flip()
p + ggsave('Downloads/boxplot_group1.pdf', height = 50, width = 50, device = 'pdf', limitsize = FALSE)

BENCH <- runtime_data[23:42,1]
BENCH <- cbind(BENCH,(runtime_data[23:42,6:9] / runtime_data[23:42,2:5]))
metric = gather(BENCH, key='id', value='value', 2, 3, 4, 5)
p = ggplot() + geom_boxplot(data= metric, aes(BENCH, log(value)), colour='black') + coord_flip()
p + ggsave('Downloads/boxplot_group2.pdf', height = 50, width = 50, device = 'pdf', limitsize = FALSE)

BENCH <- runtime_data[43:66,1]
BENCH <- cbind(BENCH,(runtime_data[43:66,6:9] / runtime_data[43:66,2:5]))
metric = gather(BENCH, key='id', value='value', 2, 3, 4, 5)
p = ggplot() + geom_boxplot(data= metric, aes(BENCH, log(value)), colour='black') + coord_flip()
p + ggsave('Downloads/boxplot_group3.pdf', height = 50, width = 50, device = 'pdf', limitsize = FALSE)

BENCH <- runtime_data[,1]
BENCH <- cbind(BENCH,(runtime_data[,6:9] / runtime_data[,2:5]))
metric = gather(BENCH, key='id', value='value', 2, 3, 4, 5)
p = ggplot() + geom_boxplot(data= metric, aes(BENCH, log(value)), colour='black') + coord_flip()  +
  theme(text=element_text(size=8,  family="Comic Sans MS"))
p + ggsave('Downloads/boxplot.pdf', height = 50, width = 50, device = 'pdf', limitsize = FALSE) 

BENCH <- runtime_data[,1]
BENCH <- cbind(BENCH,(runtime_data[,6:9] / runtime_data[,2:5]))
metric = gather(BENCH, key='id', value='value', 2, 3, 4, 5)
p = ggplot() + geom_boxplot(data= metric, aes(BENCH, log(value)), colour='black') + 
  theme(text=element_text(size=8,  family="Comic Sans MS"))
p + ggsave('Downloads/boxplot2.pdf', height = 50, width = 50, device = 'pdf', limitsize = FALSE) 
